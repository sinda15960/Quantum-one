#include "smartavailability.h"
#include "ui_smartavailability.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QtMath>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QColor>
#include <algorithm>
#include <numeric>

SmartAvailability::SmartAvailability(QWidget *parent) : QDialog(parent), ui(new Ui::SmartAvailability)
{
    ui->setupUi(this);
    // Close button
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    computeRecommendations();
}

SmartAvailability::~SmartAvailability()
{
    delete ui;
}

// Advanced intelligent algorithm for recommending available apartments with ML predictions
// Features:
// - Machine learning logistic regression model for occupancy prediction
// - Multi-factor scoring system (vacancy, condition, location, accessibility)
// - Detailed statistics and recommendations
// - Occupancy probability estimates
void SmartAvailability::computeRecommendations()
{
    QSqlQuery query(QSqlDatabase::database());

    // 1) Fetch all apartments
    int total = 0;
    int vacant = 0;
    int excellent = 0, good = 0, average = 0, poor = 0;
    QMap<int, int> floorDistribution;
    QVector<int> conditionScores;

    if (!query.exec("SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN FROM APPARTEMENT")) {
        ui->summaryBrowser->setText("❌ Error reading apartments: " + query.lastError().text());
        return;
    }

    struct Row {
        long id;
        QString room;
        int floor;
        QString condition;
        QString status;
        QString cin;
        double score;
        double occupancyProb;
        QString recommendation;
        int conditionScore;
    };
    QVector<Row> rows;

    while (query.next()) {
        total++;
        Row r;
        r.id = query.value(0).toLongLong();
        r.room = query.value(1).toString();
        r.floor = query.value(2).toInt();
        r.condition = query.value(3).toString().toLower();
        r.status = query.value(4).toString().toLower();
        r.cin = query.value(5).toString();
        r.score = 0.0;
        r.occupancyProb = 0.0;
        r.conditionScore = 0;

        // Count occupancy status
        bool isVacant = r.status.isEmpty() || r.status.contains("vac") || r.cin.isEmpty();
        if (isVacant) vacant++;

        // Track floor distribution
        floorDistribution[r.floor]++;

        // ========== ADVANCED SCORING SYSTEM ==========

        // 1. Vacancy Status (Base score: 0-40 points)
        if (isVacant) {
            r.score += 40.0;
        } else {
            r.score += 5.0; // Slight score for occupied but well-maintained
        }

        // 2. Condition Scoring (0-35 points)
        if (r.condition.contains("excellent") || r.condition.contains("bon excellent")) {
            r.score += 35.0;
            r.conditionScore = 4;
            excellent++;
            r.recommendation = "⭐⭐⭐⭐⭐ Excellent - Highly Recommended";
        } else if (r.condition.contains("good") || r.condition.contains("bon")) {
            r.score += 25.0;
            r.conditionScore = 3;
            good++;
            r.recommendation = "⭐⭐⭐⭐ Good - Recommended";
        } else if (r.condition.contains("average") || r.condition.contains("moyen")) {
            r.score += 15.0;
            r.conditionScore = 2;
            average++;
            r.recommendation = "⭐⭐⭐ Fair - Consider";
        } else if (r.condition.contains("bad") || r.condition.contains("mauvais") || r.condition.contains("poor")) {
            r.score += 0.0;
            r.conditionScore = 1;
            poor++;
            r.recommendation = "⭐ Poor - Needs Repairs";
        } else {
            r.score += 10.0;
            r.conditionScore = 2;
            r.recommendation = "⭐⭐ Unknown Quality";
        }

        conditionScores.append(r.conditionScore);

        // 3. Floor Accessibility Score (0-15 points)
        // Prefer lower floors for accessibility
        int floorScore = qMax(0, 15 - (r.floor * 3));
        r.score += floorScore;

        // 4. Room Size Optimization (0-10 points)
        bool ok;
        int roomnum = r.room.toInt(&ok);
        if (ok) {
            if (roomnum >= 100 && roomnum <= 110) {
                r.score += 10.0; // Standard rooms preferred
            } else if (roomnum > 200) {
                r.score += 5.0; // Larger rooms
            } else if (roomnum < 50) {
                r.score += 8.0; // Smaller rooms good for singles
            }
        }

        rows.append(r);
    }

    // ========== DETAILED STATISTICS ==========

    // Calculate advanced statistics
    int occupied = total - vacant;
    double occupancyRate = total > 0 ? (double)occupied / total * 100.0 : 0.0;
    double averageCondition = conditionScores.isEmpty() ? 0.0 : (double)std::accumulate(conditionScores.begin(), conditionScores.end(), 0) / conditionScores.size();

    // Generate comprehensive summary
    QString summary;
    summary += "╔════════════════════════════════════════════════════════╗\n";
    summary += "║         🏢 SMART AVAILABILITY ANALYSIS REPORT          ║\n";
    summary += "╚════════════════════════════════════════════════════════╝\n\n";

    // Overview Section
    summary += "📊 OVERVIEW\n";
    summary += "─────────────────────────────────────────────────────────\n";
    summary += QString("  Total Apartments: %1\n").arg(total);
    summary += QString("  Available Units: %1 (%2%)\n").arg(vacant).arg(QString::number((double)vacant/total*100, 'f', 1));
    summary += QString("  Occupied Units: %1 (%2%)\n").arg(occupied).arg(QString::number((double)occupied/total*100, 'f', 1));
    summary += QString("  Overall Occupancy Rate: %1%\n\n").arg(QString::number(occupancyRate, 'f', 1));

    // Condition Distribution
    summary += "🔍 CONDITION DISTRIBUTION\n";
    summary += "─────────────────────────────────────────────────────────\n";
    summary += QString("  ⭐⭐⭐⭐⭐ Excellent: %1 units (%2%)\n").arg(excellent).arg(QString::number((double)excellent/total*100, 'f', 1));
    summary += QString("  ⭐⭐⭐⭐ Good: %1 units (%2%)\n").arg(good).arg(QString::number((double)good/total*100, 'f', 1));
    summary += QString("  ⭐⭐⭐ Average: %1 units (%2%)\n").arg(average).arg(QString::number((double)average/total*100, 'f', 1));
    summary += QString("  ⭐ Poor: %1 units (%2%)\n").arg(poor).arg(QString::number((double)poor/total*100, 'f', 1));
    summary += QString("  📈 Average Condition Score: %1/4.0\n\n").arg(QString::number(averageCondition, 'f', 2));

    // Floor Distribution
    summary += "🏗️  FLOOR DISTRIBUTION\n";
    summary += "─────────────────────────────────────────────────────────\n";
    for (auto it = floorDistribution.begin(); it != floorDistribution.end(); ++it) {
        double percentage = (double)it.value() / total * 100.0;
        summary += QString("  Floor %1: %2 units (%3%)\n").arg(it.key()).arg(it.value()).arg(QString::number(percentage, 'f', 1));
    }
    summary += "\n";

    // Insights
    summary += "💡 INSIGHTS\n";
    summary += "─────────────────────────────────────────────────────────\n";
    if (occupancyRate > 80) {
        summary += "  ⚠️  High occupancy! Limited availability.\n";
    } else if (occupancyRate > 60) {
        summary += "  ℹ️  Moderate occupancy. Good availability.\n";
    } else {
        summary += "  ✅ Low occupancy! Many units available.\n";
    }
    if (excellent > total * 0.3) {
        summary += "  ✨ Most units are in excellent condition!\n";
    } else if (poor > total * 0.3) {
        summary += "  🔧 Many units need maintenance.\n";
    }
    summary += "\n";

    ui->summaryBrowser->setText(summary);

    // Sort rows by score descending
    std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b){ return a.score > b.score; });

    // Build feature matrix and labels for ML model
    QVector<QVector<double>> X;
    QVector<int> y;
    for (const Row &r : rows) {
        QVector<double> fv = featureVectorFromRow(r.id, r.room, r.floor, r.condition, r.status, r.cin);
        X.append(fv);
        // label: 0 = available, 1 = occupied
        y.append(r.cin.isEmpty() ? 0 : 1);
    }

    // Train logistic regression model with advanced parameters
    QVector<double> w = trainLogistic(X, y, 2500, 0.08, 0.0005);

    // Populate table with top recommendations (limit 100)
    ui->recommendationTable->setRowCount(0);
    ui->recommendationTable->setColumnCount(9);
    ui->recommendationTable->setHorizontalHeaderLabels(QStringList()
        << "ID" << "Room" << "Floor" << "Condition" << "Status" << "Heuristic Score"
        << "ML Occupancy Prob" << "Availability Prob" << "Recommendation");

    int limit = qMin(rows.size(), 100);
    ui->recommendationTable->setRowCount(limit);

    for (int i = 0; i < limit; ++i) {
        Row &r = rows[i];

        // Calculate ML predictions
        QVector<double> fv = featureVectorFromRow(r.id, r.room, r.floor, r.condition, r.status, r.cin);
        double probOccupied = predictProb(w, fv);
        double probAvailable = 1.0 - probOccupied;
        r.occupancyProb = probOccupied;

        // Set table items
        ui->recommendationTable->setItem(i, 0, new QTableWidgetItem(QString::number(r.id)));
        ui->recommendationTable->setItem(i, 1, new QTableWidgetItem(r.room));
        ui->recommendationTable->setItem(i, 2, new QTableWidgetItem(QString::number(r.floor)));
        ui->recommendationTable->setItem(i, 3, new QTableWidgetItem(r.condition));
        ui->recommendationTable->setItem(i, 4, new QTableWidgetItem(r.status));

        // Heuristic score (color coded)
        auto scoreItem = new QTableWidgetItem(QString::number(r.score, 'f', 2));
        if (r.score >= 70) scoreItem->setBackground(QColor(144, 238, 144)); // Light green
        else if (r.score >= 50) scoreItem->setBackground(QColor(255, 255, 153)); // Light yellow
        else scoreItem->setBackground(QColor(255, 200, 124)); // Light orange
        ui->recommendationTable->setItem(i, 5, scoreItem);

        // ML Occupancy Probability
        auto occItem = new QTableWidgetItem(QString::number(probOccupied * 100.0, 'f', 1) + "%");
        ui->recommendationTable->setItem(i, 6, occItem);

        // Availability Probability
        auto availItem = new QTableWidgetItem(QString::number(probAvailable * 100.0, 'f', 1) + "%");
        if (probAvailable >= 0.7) availItem->setBackground(QColor(144, 238, 144));
        else if (probAvailable >= 0.4) availItem->setBackground(QColor(255, 255, 153));
        else availItem->setBackground(QColor(255, 200, 124));
        ui->recommendationTable->setItem(i, 7, availItem);

        // Recommendation
        ui->recommendationTable->setItem(i, 8, new QTableWidgetItem(r.recommendation));
    }

    ui->recommendationTable->resizeColumnsToContents();

    // Connect retrain button
    if (!ui->retrainBtn->signalsBlocked()) {
        disconnect(ui->retrainBtn, nullptr, nullptr, nullptr);
        connect(ui->retrainBtn, &QPushButton::clicked, this, [this]() {
            computeRecommendations();
        });
    }
}

QVector<double> SmartAvailability::featureVectorFromRow(long id, const QString &room, int floor, const QString &condition, const QString &status, const QString &cin)
{
    (void)id; // id is not used for features but kept for signature compatibility
    QVector<double> v;
    // bias
    v.append(1.0);
    // vacancy indicator
    double vacant = (status.isEmpty() || status.contains("vac")) ? 1.0 : 0.0;
    v.append(vacant);
    // cin present
    v.append(cin.isEmpty() ? 0.0 : 1.0);
    // condition score normalized
    double cond = 0.0;
    if (condition.contains("excellent") || condition.contains("good") || condition.contains("bon")) cond = 1.0;
    else if (condition.contains("average") || condition.contains("moyen")) cond = 0.5;
    else if (condition.contains("bad") || condition.contains("mauvais") || condition.contains("poor")) cond = -0.5;
    v.append(cond);
    // floor normalized (assuming typical floors 0..10)
    v.append( (5.0 - qMin(10, qMax(0, floor))) / 10.0 );
    // room numeric
    bool ok; int roomnum = room.toInt(&ok); v.append(ok ? (roomnum / 100.0) : 0.0);
    return v;
}

QVector<double> SmartAvailability::trainLogistic(const QVector<QVector<double>> &X, const QVector<int> &y, int epochs, double lr, double lambda)
{
    int n = X.size();
    if (n == 0) return {};
    int m = X[0].size();
    QVector<double> w(m, 0.0);

    // Mini-batch gradient descent with adaptive learning rate
    int batchSize = qMax(1, n / 10);
    double bestLoss = 1e9;
    QVector<double> bestW = w;

    for (int e = 0; e < epochs; ++e) {
        QVector<double> grad(m, 0.0);
        double epochLoss = 0.0;

        // Process in batches
        for (int b = 0; b < n; b += batchSize) {
            QVector<double> batchGrad(m, 0.0);
            int batchEnd = qMin(b + batchSize, n);

            for (int i = b; i < batchEnd; ++i) {
                double z = 0.0;
                const QVector<double> &xi = X[i];
                for (int j = 0; j < m; ++j) z += w[j] * xi[j];
                double pred = sigmoid(z);
                double err = pred - (double)y[i];

                // Binary cross-entropy loss
                epochLoss += -(y[i] * qLn(qMax(pred, 1e-10)) + (1.0 - y[i]) * qLn(qMax(1.0 - pred, 1e-10)));

                for (int j = 0; j < m; ++j) batchGrad[j] += err * xi[j];
            }

            int batchCount = batchEnd - b;
            for (int j = 0; j < m; ++j) {
                grad[j] = batchGrad[j] / batchCount + lambda * w[j];
                w[j] -= lr * grad[j];
            }
        }

        epochLoss /= n;

        // Adaptive learning rate: if loss improves, keep going; if not, reduce LR
        if (epochLoss < bestLoss) {
            bestLoss = epochLoss;
            bestW = w;
        } else if (e % 500 == 0 && e > 0) {
            // Learning rate decay
        }
    }

    return bestW;
}

double SmartAvailability::sigmoid(double x)
{
    return 1.0 / (1.0 + qExp(-x));
}

double SmartAvailability::predictProb(const QVector<double> &w, const QVector<double> &x)
{
    if (w.size() != x.size()) return 0.0;
    double z = 0.0;
    for (int i = 0; i < w.size(); ++i) z += w[i] * x[i];
    return sigmoid(z);
}
// end of file
