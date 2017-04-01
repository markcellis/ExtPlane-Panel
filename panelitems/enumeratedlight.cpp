#include "enumeratedlight.h"

#include <QLineEdit>
#include <QLabel>
#include <QGraphicsScene>
#include <QImage>
#include <QBitmap>
#include <QComboBox>

#include "../util/console.h"
#include "extplaneclient.h"



REGISTER_WITH_PANEL_ITEM_FACTORY(EnumeratedLight,"indicator/light/enumerated")

EnumeratedLight::EnumeratedLight(ExtPlanePanel *panel, ExtPlaneConnection *conn) :
        PanelItem(panel, PanelItemTypeDisplay, PanelItemShapeRectangular),
        _client(this, typeName(), conn) {

    // Init

    clearStates();
    _labelGlowItem = NULL;
    _on = false;
    loadPreset(1);

    // Make connection
    conn->registerClient(&_client);
    connect(&_client, SIGNAL(refChanged(QString,QString)), this, SLOT(dataRefChanged(QString,QString)));
    connect(&_client, SIGNAL(refChanged(QString,double)), this, SLOT(dataRefChanged(QString,double)));

    // Defaults
    setSize(200,60);
}

EnumeratedLight::~EnumeratedLight() {
    if(_labelGlowItem && this->scene()) {
        _labelGlowItem->scene()->removeItem(_labelGlowItem);
    }
}

void EnumeratedLight::clearStates(void)
{
    for(int i = 0; i < MAX_STATES; i++)
    {
        _labelOn[i] = QString("");
        _valueOn[i] = 0;
        _onMask[i] = 0;
        _labelColorOn[i] = Qt::white;
    }
}

void EnumeratedLight::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    // Init painter
    setupPainter(painter);
    painter->save(); {

        // Draw the dataref name and value
        painter->setBrush(Qt::NoBrush);
        if(_on)
        {
            painter->setPen(_labelColor);
        }
        else
        {
            painter->setPen(_labelColorOff);
        }
        QFont font = defaultFont;
        font.setBold(true);
        font.setPixelSize(height()*0.7+itemFontSize());
        painter->setFont(font);
        if(_on) {
            painter->setOpacity(_strengthOn/100.0);
            painter->drawText(QRect(0,0,width(), height()), Qt::AlignCenter, _onCurrentLabel);
        } else {
            painter->setOpacity(_strengthOff/100.0);
            painter->drawText(QRect(0,0,width(), height()), Qt::AlignCenter, _labelOff);
        }

    } painter->restore();

    // Update the glow if enabled
    if(_glowEnabled && _labelGlowItem) {
        _labelGlowItem->setVisible(_on);
    }

    PanelItem::paint(painter, option, widget);
}

void EnumeratedLight::createLabel(int w, int h) {
    if(_glowEnabled && this->scene()) {

        // Draw label glow
        QPixmap pixmap = QPixmap(w,h);
        pixmap.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&pixmap); {
            setupPainter(&painter);
            painter.setBrush(_labelColor);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(0,0,w,h,20,20);
        } painter.end();

        // Setup the graphics item for glow
        // This has a special z-value ontop of other graphics items so that it can glow above the panel cover...
        if(_labelGlowItem) {
            this->scene()->removeItem(_labelGlowItem);
        }
        _labelGlowItem = new QGraphicsPixmapItem(NULL);
        _labelGlowItem->setPixmap(pixmap);
        _labelGlowItem->setOpacity(_glowStrength/100.0);
        _labelGlowItem->setX(this->x());
        _labelGlowItem->setY(this->y());
        _labelGlowItem->setZValue(PANEL_PANELITEM_GLOW_ZVALUE);
        QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(this);
        effect->setBlurRadius(40);
        _labelGlowItem->setGraphicsEffect(effect);
        this->scene()->addItem(_labelGlowItem);
    }
    else if(_labelGlowItem && this->scene())
    {
        this->scene()->removeItem(_labelGlowItem);        // Remove a prevously created one
        _labelGlowItem = NULL;
    }
}

void EnumeratedLight::itemSizeChanged(float w, float h) {
    createLabel(w,h);
}

void EnumeratedLight::settingChanged() {
    evaluateDatarefValue();
    update();
}

void EnumeratedLight::setPos(int x, int y) {
    PanelItem::setPos(x,y);
    if(_glowEnabled && _labelGlowItem) {
        _labelGlowItem->setX(x);
        _labelGlowItem->setY(y);
    }
}

void EnumeratedLight::storeSettings(QSettings &settings) {
    PanelItem::storeSettings(settings);


    settings.setValue("datarefname", _datarefName);
    settings.setValue("strengthon", _strengthOn);
    settings.setValue("strengthoff", _strengthOff);
    settings.setValue("offvalue", _valueOff);
    settings.setValue("offmask", _offMask);
    settings.setValue("offcolor", _labelColorOff);
    settings.setValue("labeloff", _labelOff);
    settings.setValue("glowstrength", _glowStrength);

    QString settingLabel;

    for(int i = 0; i < MAX_STATES; i++)
    {
        settingLabel.sprintf("onvalue%d",i);
        settings.setValue(settingLabel, _valueOn[i]);

        settingLabel.sprintf("onmask%d",i);
        settings.setValue(settingLabel, _onMask[i]);

        settingLabel.sprintf("onlabel%d",i);
        settings.setValue(settingLabel, _labelOn[i]);

        settingLabel.sprintf("oncolor%d",i);
        settings.setValue(settingLabel, _labelColorOn[i]);

    }



}

void EnumeratedLight::loadSettings(QSettings &settings) {
    PanelItem::loadSettings(settings);

    setDataRefName(settings.value("datarefname","sim/cockpit/misc/compass_indicated").toString());
    setStrengthOn(settings.value("strengthon","100").toInt());
    setStrengthOff(settings.value("strengthoff","20").toInt());

    setGlowStrength(settings.value("glowstrength","80").toInt());

    setValueOff(settings.value("offvalue","0").toInt());
    setMaskOff(settings.value("offmask","0").toInt());
    setLabelOff(settings.value("labeloff","BRAKES").toString());
    setLabelColorOff(QColor(settings.value("offcolor","red").toString()));

    QString settingLabel;

    for(int i = 0; i < MAX_STATES; i++)
    {
        settingLabel.sprintf("onvalue%d",i);
        setValueOn(i,settings.value(settingLabel,"0").toInt());

        settingLabel.sprintf("onmask%d",i);
        setMaskOn(i,settings.value(settingLabel,"0").toInt());

        settingLabel.sprintf("onlabel%d",i);
        setLabelOn(i,settings.value(settingLabel,"").toString());

        settingLabel.sprintf("oncolor%d",i);
        setColorOn(i,QColor(settings.value(settingLabel,"").toString()));

    }

    settingChanged();
    DEBUG << _datarefName;
}

void EnumeratedLight::createSettings(QGridLayout *layout) {
    PanelItem::createSettings(layout);

    // Standard settings
    createLineEditSetting(layout,"DataRef",_datarefName,SLOT(setDataRefName(QString)));
    createSliderSetting(layout,"Strength On",0,100,_strengthOn,SLOT(setStrengthOn(int)));
    createSliderSetting(layout,"Strength Off",0,100,_strengthOff,SLOT(setStrengthOff(int)));

    createNumberInputSetting(layout,"Off Value", _valueOff,SLOT(setValueOff(float)));
    createNumberInputSetting(layout,"Off Mask", _offMask,SLOT(setMaskOff(float)));
    createLineEditSetting(layout,"Label Off",_labelOff,SLOT(setLabelOff(QString)));
    createColorSetting(layout,"Off Label Color",_labelColorOff,SLOT(setLabelColorOff(QColor)));


    createSliderSetting(layout,"Label Glow",0,100,_glowStrength,SLOT(setGlowStrength(int)));

    createNumberInputSetting(layout,"On Value 1", _valueOn[0],SLOT(setValueOn0(float)));
    createNumberInputSetting(layout,"On Mask 1", _onMask[0],SLOT(setMaskOn0(float)));
    createLineEditSetting(layout,"On Label 1", _labelOn[0],SLOT(setLabelOn0(QString)));
    createColorSetting(layout,"On Label Color 1",_labelColorOn[0],SLOT(setLabelColorOn0(QColor)));

    createNumberInputSetting(layout,"On Value 2", _valueOn[1],SLOT(setValueOn1(float)));
    createNumberInputSetting(layout,"On Mask 2", _onMask[1],SLOT(setMaskOn1(float)));
    createLineEditSetting(layout,"On Label 2", _labelOn[1],SLOT(setLabelOn1(QString)));
    createColorSetting(layout,"On Label Color 2",_labelColorOn[1],SLOT(setLabelColorOn1(QColor)));

    createNumberInputSetting(layout,"On Value 3", _valueOn[2],SLOT(setValueOn2(float)));
    createNumberInputSetting(layout,"On Mask 3", _onMask[2],SLOT(setMaskOn2(float)));
    createLineEditSetting(layout,"On Label 3", _labelOn[2],SLOT(setLabelOn2(QString)));
    createColorSetting(layout,"On Label Color 3",_labelColorOn[2],SLOT(setLabelColorOn2(QColor)));

    createNumberInputSetting(layout,"On Value 4", _valueOn[3],SLOT(setValueOn3(float)));
    createNumberInputSetting(layout,"On Mask 4", _onMask[3],SLOT(setMaskOn3(float)));
    createLineEditSetting(layout,"On Label 4", _labelOn[3],SLOT(setLabelOn3(QString)));
    createColorSetting(layout,"On Label Color 4",_labelColorOn[3],SLOT(setLabelColorOn3(QColor)));

    createNumberInputSetting(layout,"On Value 5", _valueOn[4],SLOT(setValueOn4(float)));
    createNumberInputSetting(layout,"On Mask 5", _onMask[4],SLOT(setMaskOn4(float)));
    createLineEditSetting(layout,"On Label 5", _labelOn[4],SLOT(setLabelOn4(QString)));
    createColorSetting(layout,"On Label Color 5",_labelColorOn[4],SLOT(setLabelColorOn4(QColor)));



    // Presets

    layout->addWidget(new QLabel("Load Preset", layout->parentWidget()));
    QComboBox *combobox = new QComboBox(layout->parentWidget());
    combobox->addItem("");
    combobox->addItem("HSI Source");
    combobox->addItem("Autopilot ALT");
    combobox->addItem("Autopilot NAV");
    combobox->addItem("Autopilot APR");
    combobox->addItem("Autopilot HDG");
    combobox->addItem("Autopilot ATHR");
    combobox->addItem("Autopilot VS");
    combobox->addItem("Autopilot IAS");
    combobox->addItem("Autopilot FLCH");
    combobox->addItem("Autopilot VNAV");
    combobox->addItem("Transponder Mode");

    layout->addWidget(combobox);
    connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(loadPreset(int)));

    connect(combobox, SIGNAL(currentIndexChanged(int)), layout->parentWidget()->window(), SLOT(close())); // This is kindof a hack, but we need to close the window to reflect the changes to the gui

}

void EnumeratedLight::loadPreset(int val) {

// Autopilot state bit masks
#define AUTOTHROTTLE_ENGAGE (1)
#define HEADING_HOLD_ENGAGE (2)
#define ALTITUDE_HOLD_ARM (32)
#define ALTITUDE_HOLD_ENGAGED (16384)
#define HNAV_ARMED (256)
#define HNAV_ENGAGED (512)
#define GLIDESLOPE_ARMED (1024)
#define GLIDESLOPE_ENGAGED (2048)
#define VVI_CLIMB_ENGAGED (16)
#define AIRSPEED_HOLD (8)
#define FLIGHT_LEVEL_CHANGE_ENGAGE (64)
#define VNAV_ENGAGED (4096)

    clearStates();

    if(val == 1) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit2/radios/actuators/HSI_source_select_pilot");

        _labelOff = "HSI SRC";
        _valueOff = -1;
        _offMask = 0;
        _labelColorOff = Qt::yellow;

        _labelOn[0] = "NAV1";
        _labelOn[1] = "NAV2";
        _labelOn[2] = "GPS";
        _valueOn[0] = 0;
        _valueOn[1] = 1;
        _valueOn[2] = 2;
        _onMask[0] = 0;
        _onMask[1] = 0;
        _onMask[2] = 0;
        _labelColorOn[0] = Qt::green;
        _labelColorOn[1] = Qt::yellow;
        _labelColorOn[2] = Qt::blue;

    } else if(val == 2) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "ALT";
        _valueOff = 0;
        _offMask = ALTITUDE_HOLD_ARM | ALTITUDE_HOLD_ENGAGED;
        _labelColorOff = Qt::white;

        _labelOn[0] = "ALT ARM";
        _labelOn[1] = "ALT ENG";
        _valueOn[0] = ALTITUDE_HOLD_ARM;
        _valueOn[1] = ALTITUDE_HOLD_ENGAGED;
        _onMask[0] = ALTITUDE_HOLD_ARM;
        _onMask[1] = ALTITUDE_HOLD_ENGAGED;
        _labelColorOn[0] = Qt::yellow;
        _labelColorOn[1] = Qt::green;

    } else if(val == 3) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "NAV";
        _valueOff = 0;
        _offMask = HNAV_ARMED | HNAV_ENGAGED;
        _labelColorOff = Qt::white;

        _labelOn[0] = "NAV ARM";
        _labelOn[1] = "NAV ENG";
        _valueOn[0] = HNAV_ARMED;
        _valueOn[1] = HNAV_ENGAGED;
        _onMask[0] = HNAV_ARMED;
        _onMask[1] = HNAV_ENGAGED;
        _labelColorOn[0] = Qt::yellow;
        _labelColorOn[1] = Qt::green;

    } else if(val == 4) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "APR";
        _valueOff = 0;
        _offMask = GLIDESLOPE_ARMED | GLIDESLOPE_ENGAGED;
        _labelColorOff = Qt::white;

        _labelOn[0] = "APR ARM";
        _labelOn[1] = "APR ENG";
        _valueOn[0] = GLIDESLOPE_ARMED;
        _valueOn[1] = GLIDESLOPE_ENGAGED;
        _onMask[0] = GLIDESLOPE_ARMED;
        _onMask[1] = GLIDESLOPE_ENGAGED;
        _labelColorOn[0] = Qt::yellow;
        _labelColorOn[1] = Qt::green;

    } else if(val == 5) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "HDG";
        _valueOff = 0;
        _offMask = HEADING_HOLD_ENGAGE;
        _labelColorOff = Qt::white;

        _labelOn[0] = "HDG ENG";

        _valueOn[0] = HEADING_HOLD_ENGAGE;

        _onMask[0] = HEADING_HOLD_ENGAGE;
        _labelColorOn[0] = Qt::green;


    } else if(val == 6) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "ATHR";
        _valueOff = 0;
        _offMask = AUTOTHROTTLE_ENGAGE;
        _labelColorOff = Qt::white;

        _labelOn[0] = "ATHR ENG";

        _valueOn[0] = AUTOTHROTTLE_ENGAGE;

        _onMask[0] = AUTOTHROTTLE_ENGAGE;
        _labelColorOn[0] = Qt::green;

    } else if(val == 7) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "VS";
        _valueOff = 0;
        _offMask = VVI_CLIMB_ENGAGED;
        _labelColorOff = Qt::white;

        _labelOn[0] = "VS ENG";

        _valueOn[0] = VVI_CLIMB_ENGAGED;

        _onMask[0] = VVI_CLIMB_ENGAGED;
        _labelColorOn[0] = Qt::green;

    } else if(val == 8) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "IAS";
        _valueOff = 0;
        _offMask = AIRSPEED_HOLD;
        _labelColorOff = Qt::white;

        _labelOn[0] = "IAS ENG";

        _valueOn[0] = AIRSPEED_HOLD;

        _onMask[0] = AIRSPEED_HOLD;
        _labelColorOn[0] = Qt::green;

    } else if(val == 9) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "FLCH";
        _valueOff = 0;
        _offMask = FLIGHT_LEVEL_CHANGE_ENGAGE;
        _labelColorOff = Qt::white;

        _labelOn[0] = "FLCH ENG";

        _valueOn[0] = FLIGHT_LEVEL_CHANGE_ENGAGE;

        _onMask[0] = FLIGHT_LEVEL_CHANGE_ENGAGE;
        _labelColorOn[0] = Qt::green;

    } else if(val == 10) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit/autopilot/autopilot_state");

        _labelOff = "VNAV";
        _valueOff = 0;
        _offMask = VNAV_ENGAGED;
        _labelColorOff = Qt::white;

        _labelOn[0] = "VNAV ENG";

        _valueOn[0] = VNAV_ENGAGED;

        _onMask[0] = VNAV_ENGAGED;
        _labelColorOn[0] = Qt::green;

    } else if(val == 11) {
        _strengthOn = 100;
        _strengthOff = 20;
        setGlowStrength(80);
        setDataRefName("sim/cockpit2/radios/actuators/transponder_mode");

        _labelOff = "Off";
        _valueOff = 0;
        _offMask = 0;
        _labelColorOff = Qt::white;

        _labelOn[0] = "STBDY";
        _valueOn[0] = 1;
        _onMask[0] = 0;
        _labelColorOn[0] = Qt::yellow;

        _labelOn[1] = "ON";
        _valueOn[1] = 2;
        _onMask[1] = 0;
        _labelColorOn[1] = Qt::green;

        _labelOn[2] = "TA/RA";
        _valueOn[2] = 3;
        _onMask[2] = 0;
        _labelColorOn[2] = Qt::green;

        _labelOn[3] = "TEST";
        _valueOn[3] = 4;
        _onMask[3] = 0;
        _labelColorOn[3] = Qt::yellow;
    }




    evaluateDatarefValue();

}

void EnumeratedLight::setDataRefName(QString name) {

    // Unsubscribe old
    if(_datarefName != "" && _client.isDataRefSubscribed(_datarefName)) _client.unsubscribeDataRef(_datarefName); //TODO: there seems to be something wrong with unsubscribing...
    _datarefName = name;
     _datarefValue = 0;

    // Subscribe new
    _client.subscribeDataRef(name, 0);
    evaluateDatarefValue();
}

void EnumeratedLight::dataRefChanged(QString name, QString val) {
    dataRefChanged(name,val.toDouble());
}

void EnumeratedLight::evaluateDatarefValue(void)
{
    int maskedValue;
    bool newOn;

    if(_offMask != 0)
    {
        maskedValue = _offMask & _datarefValue;
    }
    else
    {
        maskedValue = _datarefValue;
    }

    if(maskedValue == _valueOff)
    {
        newOn = false;
    }
    else
    {
        newOn = true;

        bool foundAnOnValue = false;
        for(int i = 0; i < MAX_STATES; i++)
        {
            if(_onMask[i] != 0)
            {
                maskedValue = _onMask[i] & _datarefValue;
            }
            else
            {
                maskedValue = _datarefValue;
            }

            if(maskedValue == _valueOn[i])
            {
                foundAnOnValue = true;
                _onCurrentLabel = _labelOn[i];
                _labelColor = _labelColorOn[i];
                break;
            }

        }
        if(!foundAnOnValue)
        {
            _onCurrentLabel = QString("?%1").arg(_datarefValue);
            _labelColor = Qt::red;
        }

    }


    if(!this->panel()->hasAvionicsPower)
    {
        newOn = false;
    }
    if(_on != newOn)
    {
        _on = newOn;
    }
    createLabel(width(),height());
    update();
}

void EnumeratedLight::dataRefChanged(QString name, double val) {
    if(name != _datarefName) return;

    // On or off?
    DEBUG << name << val;
    _datarefValue = (int)val;
    evaluateDatarefValue();


}
