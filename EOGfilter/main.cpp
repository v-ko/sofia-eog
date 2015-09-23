#include <QCoreApplication>
#include <QCommandLineParser>

#include "sumfilter.h"

//--------------------------------------------------------

int main(int argc, char *argv[])
{
    //Frequency above witch stuff wont matter
    QCoreApplication app(argc, argv);
    //QTextStream qout(stderr);

    app.setApplicationName("eogfilter");
    app.setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Filter EOG data stored in .raw audio format.");
    parser.addHelpOption();
    parser.addVersionOption();

    // Input sample rate option
    QCommandLineOption inputSampleRateOption(QStringList() << "i" << "input-sample-rate",
            app.translate("main", "Set the input sample rate to <input-sample-rate>."),
            app.translate("main", "input-sample-rate"));
    parser.addOption(inputSampleRateOption);

    // Output sample rate option
    QCommandLineOption outputSampleRateOption(QStringList() << "o" << "output-sample-rate",
            app.translate("main", "Set the output sample rate to <output-sample-rate>."),
            app.translate("main", "output-sample-rate"));
    parser.addOption(outputSampleRateOption);

    // Cutoff freqency option
    QCommandLineOption cutoffFrequencyOption(QStringList() << "c" << "cutoff-frequency",
            app.translate("main", "Set the output sample rate to <cutoff-frequency>."),
            app.translate("main", "cutoff-frequency"));
    parser.addOption(cutoffFrequencyOption);

    // Flush of the buffer frequency
    QCommandLineOption flushOfTheBufferFrequencyOption(QStringList() << "f" << "flush-frequency",
            app.translate("main", "Set the frequency of flushing the buffer to <flush-frequency>."),
            app.translate("main", "flush-frequency"));
    parser.addOption(flushOfTheBufferFrequencyOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    float inputSampleRate = parser.value(inputSampleRateOption).toFloat();
    float outputSampleRate = parser.value(outputSampleRateOption).toFloat();
    float cutoffFrequency = parser.value(cutoffFrequencyOption).toFloat();
    float flushOfTheBufferFrequency = parser.value(flushOfTheBufferFrequencyOption).toFloat();

    SumFilter sumFilter(&app, inputSampleRate,outputSampleRate,cutoffFrequency,flushOfTheBufferFrequency);

    return app.exec();
}
