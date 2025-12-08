#ifndef SMARTAVAILABILITY_H
#define SMARTAVAILABILITY_H

#include <QDialog>

namespace Ui {
class SmartAvailability;
}

class SmartAvailability : public QDialog
{
    Q_OBJECT

public:
    explicit SmartAvailability(QWidget *parent = nullptr);
    ~SmartAvailability();

private:
    Ui::SmartAvailability *ui;
    void computeRecommendations();
    // prediction helpers
    QVector<double> featureVectorFromRow(long id, const QString &room, int floor, const QString &condition, const QString &status, const QString &cin);
    QVector<double> trainLogistic(const QVector<QVector<double>> &X, const QVector<int> &y, int epochs = 2000, double lr = 0.1, double lambda = 0.001);
    double sigmoid(double x);
    double predictProb(const QVector<double> &w, const QVector<double> &x);
};

#endif // SMARTAVAILABILITY_H

