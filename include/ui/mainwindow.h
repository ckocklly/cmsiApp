#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QPixmap>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void uploadFile(int category);

private:
    QString uploadDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/uploads";
    QString m_filePaths[4];
    const QString outputNames[4] {"upper_jaw", "lower_jaw", "front_ceph", "lat_ceph"};
    enum Category {UPPER_JAW, LOWER_JAW, FRONT_CEPH, LAT_CEPH};
    Ui::MainWindow *ui;
    // void onCalculateClicked();
    // void calculate();
    bool eventFilter(QObject *obj, QEvent *event);
};
