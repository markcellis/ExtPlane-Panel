#ifndef INDICATORDISPLAY_H
#define INDICATORDISPLAY_H

#include "displayinstrument.h"
#include "../units.h"

/**
 * Indicator/Label for Glass Cockpit.
 */
class IndicatorDisplay : public DisplayInstrument
{
    Q_OBJECT
public:
    Q_INVOKABLE IndicatorDisplay(ExtPlanePanel *panel, ExtPlaneConnection *conn);
    virtual void render(QPainter *painter, int width, int height);
    virtual QString typeName();
    virtual void storeSettings(QSettings &settings);
    virtual void loadSettings(QSettings &settings);
    virtual void createSettings(QGridLayout *layout);
    virtual void itemSizeChanged(float w, float h);

private:


signals:

public slots:
    void dataRefChanged(QString name, double val);
    void dataRefChanged(QString name, QString val);
    void setDataRefName(QString name);
    void setThreshold(float val) { _threshold = val; }
    void setLabelColor(QColor val) { _labelColor = val; }
    void setValueColor(QColor val) { _valueColor = val; }
    void setValueDivisor(int val) { _valueDivisor = val; }
    void setValueDivisor(float val) { _valueDivisor = val; }
    void setLabelOn(QString val) { _labelOn = val; }
    void setLabelOff(QString val) { _labelOff = val; }
    void setStrengthOn(int val) { _strengthOn = val; }
    void setStrengthOff(int val) { _strengthOff = val; }
    void setShowValue(bool val) { _showValue = val; }
    void setShowFloat(bool val) { _showFloat = val; }
    void setFloatFormat(QString name);
    void loadPreset(int val);

private:

    void update(void);

    // Internal variables
    //ExtPlaneClient _client;
    //double _datarefValue;
    bool _on;
    QString _datarefValue;
    QString _floatFormat;

    // Configuration variables
    QString _labelOn;
    QString _labelOff;
    bool _showValue;
    bool _showFloat;
    QColor _labelColor;
    QColor _valueColor;
    int _valueDivisor;
    QString _datarefName;
    double _threshold;
    int _strengthOn;
    int _strengthOff;
    double _datarefFloatValue;

};



#endif // INDICATORDISPLAY_H
