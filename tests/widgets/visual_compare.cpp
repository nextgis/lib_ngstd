/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Deterministic visual regression comparison for widget tests
 *****************************************************************************/
#include <QImage>
#include <QString>
#include <QtGlobal>

#include <cmath>
#include <cstdio>

class VisualComparator final
{
public:
    static int run(const QString &baselinePath, const QString &actualPath,
                   double maximumError)
    {
        const QImage baseline =
            QImage(baselinePath).convertToFormat(QImage::Format_RGBA8888);
        const QImage actual =
            QImage(actualPath).convertToFormat(QImage::Format_RGBA8888);
        if (baseline.isNull() || actual.isNull()) {
            std::fprintf(stderr, "Unable to load visual regression images.\n");
            return 2;
        }
        if (baseline.size() != actual.size()) {
            std::fprintf(stderr,
                         "Visual regression size mismatch: %dx%d != %dx%d.\n",
                         baseline.width(), baseline.height(), actual.width(),
                         actual.height());
            return 3;
        }

        long double squaredError = 0.0L;
        const qsizetype byteCount = baseline.sizeInBytes();
        const uchar *baselineData = baseline.constBits();
        const uchar *actualData = actual.constBits();
        for (qsizetype index = 0; index < byteCount; ++index) {
            const long double difference =
                static_cast<long double>(baselineData[index]) -
                static_cast<long double>(actualData[index]);
            squaredError += difference * difference;
        }
        const double normalizedError =
            std::sqrt(static_cast<double>(squaredError / byteCount)) / 255.0;
        std::fprintf(stdout, "Visual RMSE: %.6f (maximum %.6f).\n",
                     normalizedError, maximumError);
        return normalizedError <= maximumError ? 0 : 1;
    }
};

int main(int argumentCount, char *argumentValues[])
{
    if (argumentCount != 4) {
        std::fprintf(stderr,
                     "Usage: visual_compare BASELINE ACTUAL MAXIMUM_RMSE\n");
        return 2;
    }
    bool validThreshold = false;
    const double maximumError =
        QString::fromLocal8Bit(argumentValues[3]).toDouble(&validThreshold);
    if (!validThreshold || maximumError < 0.0) {
        std::fprintf(stderr, "Invalid maximum RMSE.\n");
        return 2;
    }
    return VisualComparator::run(QString::fromLocal8Bit(argumentValues[1]),
                                 QString::fromLocal8Bit(argumentValues[2]),
                                 maximumError);
}
