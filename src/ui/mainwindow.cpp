#include "ui/mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800, 600);
    ui->mainLayout->setAlignment(Qt::AlignTop);
    for (auto lcd : findChildren<QLCDNumber*>())
        lcd->display("---");

        
    ui->p4Verdict->setCurrentIndex(0);
    ui->p1Pages->setCurrentIndex(0);
    connect(ui->p1Combo,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        ui->p1Pages,
        &QStackedWidget::setCurrentIndex
    );

    connect(ui->calcButton, &QPushButton::clicked,
        this, &MainWindow::onCalculateClicked);
    
    for (auto spin : findChildren<QAbstractSpinBox*>())
        spin->installEventFilter(this);

    for (auto combo : findChildren<QComboBox*>())
        combo->installEventFilter(this);

}   

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        if (qobject_cast<QAbstractSpinBox*>(obj) ||
            qobject_cast<QComboBox*>(obj)) {
            event->ignore();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onCalculateClicked() {
    if (ui->caseIdEdit->text().isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Information: Case ID",
            "Please provide the case ID."
        );
        return;
    }
    if (ui->technicianEdit->text().isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Information: Technician",
            "Please provide the technician's name."
        );
        return;
    }
    if (ui->p1Combo->currentIndex() == 0) {
        QMessageBox::warning(
            this,
            "Missing Information: Section I",
            "Select a skeletal assessment method and complete Section I."
        );
        return;
    }
    if (ui->p1Combo->currentIndex() == 2 && !ui->comboBox_molar->currentIndex()) {
        QMessageBox::warning(
            this,
            "Missing Information: Section I",
            "Select a molar relationship and complete Section I."
        );
        return;
    }
    
    calculate();
    ui->p4Verdict->setCurrentIndex(1);
}

void MainWindow::calculate() {
    int score[] {0, 0};

    // Section I
    QString optAStmts[] {"", "", "", ""};
    QString optBStmt;

    if (ui->p1Combo->currentIndex() == 1) {
        int v;
        int anbScore = 0;
        QString anbFlag = "Green";
        v = ui->spin_01_anb->value();
        if (v >= 8 || v <= -5) {
            anbScore = 15;
            anbFlag = "Red";
            optAStmts[0] = "STOP: ANB indicates skeletal discrepancy - Cephalometric surgical evaluation required";
        } else if (v >= 6 || v <= -2) {
            anbScore = 8;
            anbFlag = "Yellow";
        }
        ui->lcd_01_anb->display(anbScore);
        ui->flag_01_anb->setText(anbFlag);
            
        v = ui->spin_02_snmp->value();
        int snmpScore = 0;
        QString snmpFlag = "Green";
        if (v >= 40 || v <= 21) {
            snmpScore = 12;
            snmpFlag = "Red";
            optAStmts[1] = "STOP: High SN-MP angle - Check for open bite. If >2mm, closure contraindicated";
        } else if (v >= 38 || v <= 26) {
            snmpScore = 6;
            snmpFlag = "Yellow";
        }
        ui->lcd_02_snmp->display(snmpScore);
        ui->flag_02_snmp->setText(snmpFlag);

        v = ui->spin_03_skelAsym->value();
        int asymScore = 0;
        QString asymFlag = "Green";
        if (v >= 5) {
            asymScore = 10;
            asymFlag = "Red";
            optAStmts[2] = "STOP: Severe skeletal asymmetry - Surgical evaluation required";
        } else if (v >= 3) {
            asymScore = 6;
            asymFlag = "Yellow";
        } else if (v >= 1) {
            asymScore = 3;
            asymFlag = "Yellow";
        }
        ui->lcd_03_skelAsym->display(asymScore);
        ui->flag_03_skelAsym->setText(asymFlag);
        
        v = ui->spin_04_l1mp->value();
        int l1mpScore = 0;
        QString l1mpFlag = "Green";
        if (v >= 111) {
            optAStmts[3] = "STOP: Severe lower incisor proclination - No further proclination permitted (periodontal risk)";
        }
        if (v >= 111 || v <= 84) {
            l1mpScore = 5;
            l1mpFlag = "Red";
        } else if (v >= 106 || v <= 90) {
            l1mpScore = 4;
            l1mpFlag = "Yellow";
        } else if (v >= 99 || v <= 94) {
            l1mpScore = 2;
            l1mpFlag = "Yellow";
        }
        ui->lcd_04_l1mp->display(l1mpScore);
        ui->flag_04_l1mp->setText(l1mpFlag);

        score[0] = anbScore + snmpScore + asymScore + l1mpScore;
        ui->lcdP1total->display(score[0]);
    } else if (ui->p1Combo->currentIndex() == 2) {
        int i = ui->comboBox_molar->currentIndex();
        int optBScore = 0;
        int indicators = 0;
        if (ui->checkBox_01_abnormal->isChecked())  indicators++;
        if (ui->checkBox_02_face->isChecked())      indicators++;
        if (ui->checkBox_03_asym->isChecked())      indicators++;
        if (ui->checkBox_04_incisors->isChecked())  indicators++;
        if (i == 1) {
            optBStmt = "GREEN LIGHT - Proceed with aligner planning";
            optBScore = 5;
        } else if (i == 2 || i == 3) {
            if (indicators <= 1) {
                optBStmt = "PROCEED - Likely dentoalveolar, compromise treatment";
                optBScore = 15;
            } else if (indicators <= 3) {
                optBStmt = "CAUTION - Cephalometrics recommended";
                optBScore = 25;
            } else {
                optBStmt = "STOP - Cephalometrics REQUIRED before proceeding";
                optBScore = 35;
            }
        } else if (i == 4) {
            optBStmt = "STOP - Full cusp indicates skeletal discrepancy. Cephalometric analysis MANDATORY";
            optBScore = 50;
        } else if (i == 5) {
            optBStmt = "STOP - Surgical evaluation required";
            optBScore = 65;
        }
        score[0] = optBScore;
        ui->lcdP1total->display(score[0]);
    }
    
    // Section II
    int v2 = ui->spin_b1->value();
    int crowdScore = 0;
    QString crowdFlag = "Green";
    QString p2Stmts[] {"", "", ""};
    if (1) {
        if (v2 >= 11) {
            if (ui->p1Combo->currentIndex() == 1 && ui->spin_04_l1mp->value() >= 106)
                p2Stmts[0] = "WARNING: Severe crowding + proclined incisors - Cannot procline for space";
        }
        if (v2 >= 14) {
            crowdScore = 15;
            crowdFlag = "Red";
        } else if (v2 >= 11) {
            crowdScore = 13;
            crowdFlag = "Red";
        } else if (v2 >= 8) {
            crowdScore = 10;
            crowdFlag = "Yellow";
        } else if (v2 >= 6) {
            crowdScore = 7;
            crowdFlag = "Yellow";
        } else if (v2 >= 4) {
            crowdScore = 4;
            crowdFlag = "Green";
        } else if (v2 >= 1) {
            crowdScore = 2;
            crowdFlag = "Green";
        }
        ui->lcd_b1->display(crowdScore);
        ui->flag_b1->setText(crowdFlag);

        v2 = ui->spin_b2->value();
        int ojScore = 0;
        QString ojFlag = "Green";
        if (v2 >= 10) {
            ojScore = 10;
            ojFlag = "Red";
            p2Stmts[1] = "WARNING: Overjet ≥10mm - Verify skeletal pattern, may require cephalometrics";
        } else if (v2 >= 7) {
            ojScore = 6;
            ojFlag = "Yellow";
        } else if (v2 >= 4) {
            ojScore = 3;
            ojFlag = "Green";
        }
        ui->lcd_b2->display(ojScore);
        ui->flag_b2->setText(ojFlag);

        v2 = ui->spin_b3->value();
        int obScore = 0;
        QString obFlag = "Green";
        if (v2 <= -4) {
            p2Stmts[2] = "WARNING: Open bite ≥3mm - Check skeletal angle, may be contraindication";
        }
        if (v2 >= 8 || v2 <= -5) {
            obScore = 8;
            obFlag = "Red";
        } else if (v2 <= -3) {
            obScore = 6;
            obFlag = "Red";
        } else if (v2 >= 6) {
            obScore = 5;
            obFlag = "Yellow";
        } else if (v2 <= -1) {
            obScore = 4;
            obFlag = "Yellow";
        } else if (v2 >= 4) {
            obScore = 3;
            obFlag = "Yellow";
        }
        ui->lcd_b3->display(obScore);
        ui->flag_b3->setText(obFlag);

        int crossScore = ui->spin_b4->value() * 2;
        ui->lcd_b4->display(crossScore);
        int missingScore = ui->spin_b5->value() * 2;
        ui->lcd_b5->display(missingScore);
        int otherScore = ui->spin_b6->value();
        ui->lcd_b6->display(otherScore);

        score[1] = crowdScore + ojScore + obScore + crossScore + missingScore + otherScore;
        ui->lcdP2Total->display(score[1]);
    }

    // Section III
    QLineEdit *teethName[5] {ui->name_r1, ui->name_r2, ui->name_r3, ui->name_r4, ui->name_r5};
    QSpinBox *rotation[5] {ui->valRot_r1, ui->valRot_r2, ui->valRot_r3, ui->valRot_r4, ui->valRot_r5};
    QSpinBox *translation[5] {ui->valTrans_r1, ui->valTrans_r2, ui->valTrans_r3, ui->valTrans_r4, ui->valTrans_r5};
    QSpinBox *torque[5] {ui->valTorq_r1, ui->valTorq_r2, ui->valTorq_r3, ui->valTorq_r4, ui->valTorq_r5};
    QLabel *flag[5] {ui->flag_r1, ui->flag_r2, ui->flag_r3, ui->flag_r4, ui->flag_r5};

    int redFlagCount = 0;
    for (int i = 0; i < 5; i++) {
        QString flagText = "Green";
        if (teethName[i]->text().isEmpty()) continue;
        if (rotation[i]->value() >= 46 || translation[i]->value() >= 4 || torque[i]->value() >= 16) {
            flagText = "Red";
            redFlagCount++;
        } else if (rotation[i]->value() >= 31 || translation[i]->value() >= 3 || torque[i]->value() >= 11) {
            flagText = "Yellow";
        }
        flag[i]->setText(flagText);
    }
    ui->lcdP3Total->display(redFlagCount);

    // Decision
    int total = score[0] + score[1];
    QString title, complexity, recommendation;
    if (total <= 20) {
        title = "ACCEPT";
        complexity = "Minimal Complexity";
        recommendation = "Proceed with standard aligner treatment. Excellent prognosis.";
    } else if (total <= 40) {
        title = "ACCEPT";
        complexity = "Mild Complexity";
        recommendation = "Proceed with standard mechanics. Good prognosis.";
    } else if (total <= 65) {
        title = "ACCEPT WITH CAUTION";
        complexity = "Moderate Complexity";
        recommendation = "Auxiliaries needed (attachments, elastics, IPR). Expected timeline +30-50%.";
    } else if (total <= 90) {
        title = "CHALLENGING";
        complexity = "Severe Complexity";
        recommendation = "Consider traditional orthodontics or combined approach. Multiple refinements expected.";
    } else {
        title = "RECONSIDER";
        complexity = "Very Severe Complexity";
        recommendation = "Aligners not recommended. Consider orthognathic surgery or traditional orthodontics.";
    }

    ui->verdictTitle->setText(title);
    ui->verdictComplexity->setText(complexity);
    ui->verdictRec->setText(recommendation);

    QString movement;
    if (redFlagCount == 0) {
        movement = "Standard movement difficulty.";
    } else if (redFlagCount <= 2) {
        movement = QString::number(redFlagCount) + "difficult movements - expect 1-2 refinements (+30\% time).";
    } else if (redFlagCount <= 4) {
        movement = QString::number(redFlagCount) + "difficult movements - expect 2-3 refinements (+50\% time).";
    } else {
        movement = QString::number(redFlagCount) + "difficult movements - very complex case (+100\% time).";
    }
    ui->verdictMvmtCtnt->setText(movement);

    QVector<QString> alerts;
    if (score[0] > 40) {
        alerts.push_back("Skeletal score > 40 - Limited treatment only or surgical evaluation required");
    }
    if (redFlagCount >= 5) {
        alerts.push_back("5+ difficult movements - Reconsider treatment approach");
    }
    for (int i = 0; i < 4; i++) {
        if (optAStmts[i] != "")
            alerts.push_back(optAStmts[i]);
    }
    if (optBStmt != "")
        alerts.push_back(optBStmt);
    for (int i = 0; i < 3; i++) {
        if (p2Stmts[i] != "")
            alerts.push_back(p2Stmts[i]);
    }
    QString alertStr;
    for (int i = 0; i < alerts.length(); i++) {
        alertStr += alerts[i];
        if (i < alerts.length() - 1)
            alertStr += "\n";
    }
    if (alertStr == "")
        ui->verdictAlertCtnt->setText("(No special alerts)");
    else
        ui->verdictAlertCtnt->setText(alertStr);
}