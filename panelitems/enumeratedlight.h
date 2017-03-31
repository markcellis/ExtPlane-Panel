#ifndef ENUMERATEDLIGHT_H
#define ENUMERATEDLIGHT_H

#include "panelitem.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>

#define MAX_STATES 5      // The number of ON states we support

class EnumeratedLight : public PanelItem
{
    Q_OBJECT

public:
    Q_INVOKABLE EnumeratedLight(ExtPlanePanel *panel, ExtPlaneConnection *conn);
    virtual ~EnumeratedLight();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QString typeName();
    virtual void storeSettings(QSettings &settings);
    virtual void loadSettings(QSettings &settings);
    virtual void createSettings(QGridLayout *layout);
    virtual void itemSizeChanged(float w, float h);
    virtual void settingChanged();
    virtual void setPos(int x, int y);
    void createLabel(int w, int h);

public slots:
    void dataRefChanged(QString name, double val);
    void dataRefChanged(QString name, QString val);
    void setDataRefName(QString name);

    void setValueOff(int value) { _valueOff = value; }
    void setMaskOff(int value) { _offMask = value; }
    void setLabelOff(QString val) { _labelOff = val; }
    void setLabelColorOff(QColor val) { _labelColorOff = val; }

    void setValueOn0(int value) {_valueOn[0] = value; }
    void setMaskOn0(int value) {_onMask[0] = value; }
    void setLabelOn0(QString val) {_labelOn[0] = val; }
    void setLabelColorOn0(QColor val) {_labelColorOn[0] = val; }

    void setValueOn1(int value) {_valueOn[1] = value; }
    void setMaskOn1(int value) {_onMask[1] = value; }
    void setLabelOn1(QString val) {_labelOn[1] = val; }
    void setLabelColorOn1(QColor val) {_labelColorOn[1] = val; }

    void setValueOn2(int value) {_valueOn[2] = value; }
    void setMaskOn2(int value) {_onMask[2] = value; }
    void setLabelOn2(QString val) {_labelOn[2] = val; }
    void setLabelColorOn2(QColor val) {_labelColorOn[2] = val; }

    void setValueOn3(int value) {_valueOn[3] = value; }
    void setMaskOn3(int value) {_onMask[3] = value; }
    void setLabelOn3(QString val) {_labelOn[3] = val; }
    void setLabelColorOn3(QColor val) {_labelColorOn[3] = val; }

    void setValueOn4(int value) {_valueOn[4] = value; }
    void setMaskOn4(int value) {_onMask[4] = value; }
    void setLabelOn4(QString val) {_labelOn[4] = val; }
    void setLabelColorOn4(QColor val) {_labelColorOn[4] = val; }


    void setLabelColor(QColor val) { _labelColor = val; createLabel(width(),height()); }


    void setGlowStrength(int val) { _glowStrength = val; _glowEnabled=(_glowStrength!=0); }
    void setStrengthOn(int val) { _strengthOn = val; }
    void setStrengthOff(int val) { _strengthOff = val; }
    void loadPreset(int val);

private:
    // Internal variables
    ExtPlaneClient _client;
    int _datarefValue;
    QString _onCurrentLabel;
    bool _on;
    QGraphicsPixmapItem *_labelGlowItem;

    // Configuration variables
    int _valueOff;
    int _offMask;
    QString _labelOff;
    QColor _labelColorOff;
    QColor _labelColorOn[MAX_STATES];
    QString _labelOn[MAX_STATES];
    int _valueOn[MAX_STATES];
    int _onMask[MAX_STATES];
    QColor _labelColor;
    bool _glowEnabled;

    QString _datarefName;
    int _strengthOn;
    int _strengthOff;
    int _glowStrength;

protected:

void clearStates(void);
void evaluateDatarefValue(void);

void setValueOn(int index,int value) {_valueOn[index] = value; }
void setMaskOn(int index, int value) {_onMask[index] = value; }
void setLabelOn(int index,QString val) {_labelOn[index] = val; }
void setColorOn(int index,QColor val) {_labelColorOn[index] = val; }


};

#endif // ENUMERATEDLIGHT_H
