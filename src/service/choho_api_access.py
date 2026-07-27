import os
import time
import trimesh
import requests
import json
import urllib

# ======== Load credentials ========
if os.path.exists('../../creds.json'):
    creds = json.load(open('../../creds.json', 'r'))
    base_url = creds['base_url']
    file_server_url = creds['file_server_url']
    zh_token = creds['zh_token']
    user_group = "APIClient"
    user_id = creds['user_id']
    print("Successfully loaded credentials from creds.json")
else:
    raise RuntimeError("Credentials not found")

# ======== Set input/output paths ========
input_path = "../../data/input/basil/"
output_path = "../../data/output/basil/json/"

# ======== API access ========
def upload_file(file_name):
    ext = file_name.split('.')[-1]
    data = open(file_name, 'rb').read()
    print(f"Uploading file \"{os.path.abspath(file_name)}\"")

    resp = requests.get(file_server_url + f"/scratch/{user_group}/{user_id}/upload_url?" +
                        f"postfix={ext}", # Must specify postfix, i.e., file extension
                        headers={"X-ZH-TOKEN": zh_token}) # Get signed upload URL
    resp.raise_for_status()

    upload_url = resp.text[1:-1] # Returns a single string JSON "string", can also use json.loads(resp.text)

    resp = requests.put(upload_url, data) # No auth header is needed for uploading to the cloud storage service

    resp.raise_for_status()
    path = "/".join(urllib.parse.urlparse(upload_url).path.lstrip("/").split("/")[3:])
    urn = f"urn:zhfile:o:s:{user_group}:{user_id}:{path}"
    return urn

def run_job_and_get_results(json_call, timeout_sec):
    headers = {
        "Content-Type": "application/json",
        "X-ZH-TOKEN": zh_token
    }

    url = base_url + '/run'

    response = requests.request("POST", url, headers=headers, data=json.dumps(json_call))
    response.raise_for_status()
    create_result = response.json()
    run_id = create_result['run_id']
    print("workflow id is", run_id)

    url = base_url + f"/run/{run_id}"

    start_time = time.time()
    while time.time()-start_time < timeout_sec:
        time.sleep(0.3)
        response = requests.request("GET", url, headers=headers)
        result = response.json()
        if result['completed'] or result['failed']:
            break

    if not result['completed']:
        if result['failed']:
            raise ValueError("API failed due to " + str(result['reason_public']))
        raise TimeoutError("API timeout")
    print(f"API finished in {time.time() - start_time}s")

    url = base_url + f"/data/{run_id}"
    response = requests.request("GET", url, headers=headers)
    return response.json()

def retrieve_data(urn):
    return requests.get(
        file_server_url + f"/file/download?" + \
            urllib.parse.urlencode({"urn": urn}),
        headers={"X-ZH-TOKEN": zh_token}
    ).content

def retrieve_mesh(mesh_file_json):
    resp = requests.get(
        file_server_url + f"/file/download?" + \
            urllib.parse.urlencode({"urn": mesh_file_json['data']}),
        headers={"X-ZH-TOKEN": zh_token}
    )
    return trimesh.load(trimesh.util.wrap_as_stream(resp.content), file_type=mesh_file_json['type'])


# ======== Analyses ========
def upper_teeth(filepath):
    json_call = {
        "spec_group": "mesh-processing",
        "spec_name": "oral-denoise-prod",
        "spec_version": "1.0-snapshot",
        "user_group": user_group,
        "user_id": user_id,
        "input_data": {
            "mesh": {
                "type": filepath.split('.')[-1],
                "data": upload_file(filepath)
            },
            "jaw_type": "Upper"
        },
        "output_config": {
            "teeth_comp": {"type": "ply"}
        }
    }
    print('Performing upper jaw analysis...')
    return run_job_and_get_results(json_call, 300)

def lower_teeth(filepath):
    json_call = {
        "spec_group": "mesh-processing",
        "spec_name": "oral-denoise-prod",
        "spec_version": "1.0-snapshot",
        "user_group": user_group,
        "user_id": user_id,
        "input_data": {
            "mesh": {
                "type": filepath.split('.')[-1],
                "data": upload_file(filepath)
            },
            "jaw_type": "Lower"
        },
        "output_config": {
            "teeth_comp": {"type": "ply"}
        }
    }
    print('Performing lower jaw analysis...')
    return run_job_and_get_results(json_call, 300)

def auto_arrange(result_upper_jaw, result_lower_jaw):
    json_call = {
        "spec_group": "mesh-processing",
        "spec_name": "auto-arrange", 
        "spec_version": "1.0-snapshot",
        "user_group": user_group,
        "user_id": user_id,
        "input_data": {
            "upper_teeth_dict": result_upper_jaw["teeth_comp"],
            "upper_axis_matrix_dict": result_upper_jaw["axis"],
            "lower_teeth_dict": result_lower_jaw["teeth_comp"],
            "lower_axis_matrix_dict": result_lower_jaw["axis"],
        }
    }
    print('Performing auto arrangement...')
    return run_job_and_get_results(json_call, 500)

def case_complexity(result_upper_jaw, result_lower_jaw, result_arrangement):
    json_call = {
        "spec_group": "mesh-processing",
        "spec_name": "case-complexity-analysis", 
        "spec_version": "1.0-snapshot",
        "user_group": user_group,
        "user_id": user_id,
        "input_data": {
            "teeth_dict": {**result_upper_jaw["teeth_comp"], **result_lower_jaw["teeth_comp"]},
            "axis_dict": {**result_upper_jaw["axis"], **result_lower_jaw["axis"]},
            "transformation_dict": result_arrangement["result"]["transformation_dict"],
            "landmarks_dict": {**result_upper_jaw["landmarks"], **result_lower_jaw["landmarks"]},
            "target_out_form": "" # No function for now, pass empty string
        }
    }
    print('Performing case complexity analysis...')
    return run_job_and_get_results(json_call, 500)

def ceph(filepath):
    json_call = {
        "spec_version": "1.0-snapshot",
        "spec_name": "ceph-analysis",
        "spec_group": "ceph",
        "user_group": user_group,
        "user_id": user_id,
        "input_data": {
            "image": upload_file(filepath)
        }
    }
    print('Performing cephalometric image analysis...')
    return run_job_and_get_results(json_call, 180)

def pano(filepath):
    json_call = {
        "spec_version": "1.0-snapshot",
        "spec_name": "pano-analysis",
        "spec_group": "pano",
        "user_group": user_group,
        "user_id": user_id,
        "input_data": {
            "image": upload_file(filepath)
        }
    }
    print('Performing panoramic radiograph analysis...')
    return run_job_and_get_results(json_call, 180)


# ======== Main ========
res_upper = upper_teeth(os.path.join(input_path, "intra_upper.stl"))
res_lower = lower_teeth(os.path.join(input_path, "intra_lower.stl"))
res_arr = auto_arrange(res_upper, res_lower)
res_complex = case_complexity(res_upper, res_lower, res_arr)
res_ceph = ceph(os.path.join(input_path, "ceph.jpg"))
res_pano = pano(os.path.join(input_path, "pan.jpg"))

arranged = {
    'analyses' : {
        'auto_arrange' : res_arr,
        'case_complexity' : res_complex,
        'ceph' : res_ceph,
        'pano' : res_pano,
        'teeth_segmentation' : {
            'result' : {
                'lower' : res_lower,
                'upper' : res_upper
            }
        }
    }
}

output_file_path = os.path.join(output_path, "choho_output.json")
with open(output_file_path, 'w') as f:
    json.dump(arranged, f, indent=2, ensure_ascii=True)

print("All analyses completed. Output JSON file in \"" 
      + os.path.abspath(output_file_path) + "\".")
