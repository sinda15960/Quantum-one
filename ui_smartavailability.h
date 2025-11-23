// Auto-created simple UI for SmartAvailability dialog
#ifndef UI_SMARTAVAILABILITY_H
#define UI_SMARTAVAILABILITY_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtGui/QFont>
#include <QtCore/QStringList>

namespace Ui {
class SmartAvailability {
public:
    QWidget *centralWidget;
    QLabel *titleLabel;
    QTextBrowser *summaryBrowser;
    QTableWidget *recommendationTable;
    QPushButton *retrainBtn;
    QDialogButtonBox *buttonBox;

    void setupUi(QWidget *parent) {
        parent->setObjectName("SmartAvailability");
        parent->resize(700,480);
        QVBoxLayout *v = new QVBoxLayout(parent);
        titleLabel = new QLabel(parent);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setText("Smart Availability");
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont f = titleLabel->font(); f.setPointSize(14); f.setBold(true); titleLabel->setFont(f);
        v->addWidget(titleLabel);

        summaryBrowser = new QTextBrowser(parent);
        summaryBrowser->setObjectName("summaryBrowser");
        summaryBrowser->setMinimumHeight(90);
        v->addWidget(summaryBrowser);

        recommendationTable = new QTableWidget(parent);
        recommendationTable->setObjectName("recommendationTable");
        recommendationTable->setColumnCount(7);
        recommendationTable->setHorizontalHeaderLabels(QStringList() << "ID" << "Room" << "Floor" << "Condition" << "Status" << "Score" << "AvailProb");
        v->addWidget(recommendationTable);
        retrainBtn = new QPushButton(parent);
        retrainBtn->setObjectName("retrainBtn");
        retrainBtn->setText("Retrain Model");
        v->addWidget(retrainBtn);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, parent);
        v->addWidget(buttonBox);
    }
};
}

#endif // UI_SMARTAVAILABILITY_H
