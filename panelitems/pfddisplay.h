#ifndef PFDDISPLAY_H
#define PFDDISPLAY_H

#include "displayinstrument.h"
#include "../units.h"

/**
 * Primary Flight Display for Glass Cockpit.
 */
class PFDDisplay : public DisplayInstrument
{
    Q_OBJECT
public:
    Q_INVOKABLE PFDDisplay(ExtPlanePanel *panel, ExtPlaneConnection *conn);
    virtual void render(QPainter *painter, int width, int height);
    virtual QString typeName();
    virtual void storeSettings(QSettings &settings);
    virtual void loadSettings(QSettings &settings);
    virtual void createSettings(QGridLayout *layout);
    virtual void itemSizeChanged(float w, float h);

private:
    float value2Angle1(float value);
    float value2Angle2(float value);
    void drawAttitudeIndicator(QPainter *painter, int attitudeX, int attitudeY, int screenWidth, int screenHeight);
    void drawScaleIndicator(QPainter *painter, int screenWidth, int screenHeight, int x, int y, int w, int h, double value, double target_value,int direction);
    void drawCompass(QPainter *painter, int screenWidth, int screenHeight, int x, int y, int w, int h);
    void drawVerticalSpeed(QPainter *painter, int screenWidth, int screenHeight, int x, int y, int w, int h);
    void createCompassBackplate(int screenWidth, int screenHeight);
    void drawDiamond(QPainter *painter,float x,float y,float radius);
    void drawGpsAnnunciator(QPainter *painter, int annunciatorX, int annunciatorY, int annunciatorWidth, int annunciatorHeight);


signals:

public slots:
    void refChanged(QString name, double value);
    void refChanged(QString name, QStringList values);
    void refChanged(QString name, QString value);

    void setStyle(int val) {_style=val;}


protected:
    int _style; // 0=Generic 1=Boeing
    int _strokeSize;
    QColor _colorGround;
    QColor _colorSky;
    QColor _colorScale;
    QColor _colorStroke;
    QColor _colorWhisker;
    QColor _colorValue;
    QColor _colorYellow;
    QColor _colorGreen;

    QPen _defaultPen;
    QPen _defaultPen2x;
    QPen _diamondPen;
    QPen _markerPen;
    QFont _tickFont;
    QFont _valueFont;
    QFont _baroFont;
    QFont _annunciatorFont;

    float _attitude_size;
    float _attitude_rollValue;
    float _attitude_pitchValue;
    float _attitude_slipValue;

    float _scale_width;
    float _scale_height;

    float _airspeed_value;
    float _airspeed_target_value;
    float _airspeedaccl_value;
    float _hsi_obs_deg_mag;



    float _altitude_value;
    float _altitude_target_value;

    // For ILS Indicators
    int _hsi_has_horizontal_signal;
    int _hsi_has_vertical_signal;
    float _hsi_horizontal_dots;
    float _hsi_vertical_dots;
    int _hsi_glideslope_flag;


    QPixmap _compass_backplate;
    float _compass_size;
    float _compass_heading_value;
    float _verticalspeed_size;
    float _verticalspeed_value;
    float _verticalspeed_target_value;
    float _headingBug_value;
    float _hsi_dme_distance_nm_pilot;
    int   _metric_press;
    float _barometer_setting;
    int _autopilot_on;
    int _autopilot_state;
    int _hsi_source_select_pilot;
    int _hsi_has_dme_pilot;
    int _hsi_dme_speed_kts_pilot;
    QString _nav1_nav_id;
    QString _nav2_nav_id;
    QString _gps_nav_id;



};

#endif // PFDDISPLAY_H
