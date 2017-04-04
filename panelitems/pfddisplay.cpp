#include "pfddisplay.h"

#include <QLabel>
#include <QCheckBox>

#include "../util/console.h"

#define _USE_MATH_DEFINES
#include <math.h>

REGISTER_WITH_PANEL_ITEM_FACTORY(PFDDisplay,"display/pfd")

#define DATAREF_PITCH "sim/cockpit2/gauges/indicators/pitch_vacuum_deg_pilot"
#define DATAREF_ROLL "sim/cockpit2/gauges/indicators/roll_electric_deg_pilot"
#define DATAREF_SLIP "sim/cockpit2/gauges/indicators/slip_deg"
#define DATAREF_AIRSPEED_KTS "sim/cockpit2/gauges/indicators/airspeed_kts_pilot"
#define DATAREF_AIRSPEED_TARGET_KTS "sim/cockpit/autopilot/airspeed"
#define DATAREF_ALTITUDE_FT "sim/cockpit2/gauges/indicators/altitude_ft_pilot"
#define DATAREF_ALTITUDE_TARGET_FT "sim/cockpit/autopilot/altitude"
#define DATAREF_AIRSPEED_ACC_KTS "sim/cockpit2/gauges/indicators/airspeed_acceleration_kts_sec_pilot"
#define DATAREF_HEADING_DEG "sim/cockpit2/gauges/indicators/heading_electric_deg_mag_pilot"
#define DATAREF_VERTICALSPEED_FPM "sim/cockpit2/gauges/indicators/vvi_fpm_pilot"
#define DATAREF_VERTICALSPEED_TARGET "sim/cockpit/autopilot/vertical_velocity"
#define DATAREF_HEADING_BUG "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot"

#define DATAREF_HSI_HAS_HORIZONTAL_SIGNAL  "sim/cockpit2/radios/indicators/hsi_display_horizontal_pilot"
#define DATAREF_HSI_HAS_VERTICAL_SIGNAL "sim/cockpit2/radios/indicators/hsi_display_vertical_pilot"
#define DATAREF_HSI_HORIZONTAL_DOTS  "sim/cockpit2/radios/indicators/hsi_hdef_dots_pilot"
#define DATAREF_HSI_VERTICAL_DOTS  "sim/cockpit2/radios/indicators/hsi_vdef_dots_pilot"
#define DATAREF_HSI_GLIDESLOPE_FLAG "sim/cockpit2/radios/indicators/hsi_flag_glideslope_pilot"
#define DATAREF_METRIC_PRESS "sim/physics/metric_press"   // 0 is HG 1 is Millibars
#define DATAREF_BAROMETER_SETTING "sim/cockpit/misc/barometer_setting"
#define DATAREF_HSI_OBS_DEG_MAG "sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot"
#define DATAREF_AUTOPILOT_STATE "sim/cockpit/autopilot/autopilot_state"
#define DATAREF_AUTOPILOT_ON "sim/cockpit2/autopilot/flight_director_mode"
#define DATAREF_NAV1_NAV_ID "sim/cockpit2/radios/indicators/nav1_nav_id"
#define DATAREF_NAV2_NAV_ID "sim/cockpit2/radios/indicators/nav2_nav_id"
#define DATAREF_GPS_NAV_ID "sim/cockpit2/radios/indicators/gps_nav_id"
#define DATAREF_HSI_SOURCE_SELECT_PILOT "sim/cockpit2/radios/actuators/HSI_source_select_pilot"
#define DATAREF_HSI_HAS_DME_PILOT "sim/cockpit2/radios/indicators/hsi_has_dme_pilot"
#define DATAREF_HSI_DME_DISTANCE_NM_PILOT "sim/cockpit2/radios/indicators/hsi_dme_distance_nm_pilot"
#define DATAREF_HSI_DME_SPEED_KTS_PILOT "sim/cockpit2/radios/indicators/hsi_dme_speed_kts_pilot"

#define MILLIBARS_FACTOR 33.8639f

#define ENGINE_STYLE_GENERIC 0
#define ENGINE_STYLE_BOEING 1

#define SCALE_INDICATOR_TYPE_AIRSPEED 1
#define SCALE_INDICATOR_TYPE_ALTITUDE 2

#define ILS_BUFFER_SPACE .1f        // This is the percentage (10%) of the attiude size that can be used for the ILS dots
                                    // above and below the attiude indicator

#define AUTOTHROTTLE_ENGAGE (_autopilot_state & 1)
#define HEADING_HOLD_ENGAGE (_autopilot_state & 2)
#define ALTITUDE_HOLD_ARM (_autopilot_state & 32)
#define ALTITUDE_HOLD_ENGAGED (_autopilot_state & 16384)
#define HNAV_ARMED (_autopilot_state & 256)
#define HNAV_ENGAGED (_autopilot_state & 512)
#define GLIDESLOPE_ARMED (_autopilot_state & 1024)
#define GLIDESLOPE_ENGAGED (_autopilot_state & 2048)
#define VVI_CLIMB_ENGAGED (_autopilot_state & 16)
#define AIRSPEED_HOLD (_autopilot_state & 8)
#define FLIGHT_LEVEL_CHANGE_ENGAGE (_autopilot_state & 64)
#define VNAV_ENGAGED (_autopilot_state & 4096)






PFDDisplay::PFDDisplay(ExtPlanePanel *panel, ExtPlaneConnection *conn) :
        DisplayInstrument(panel,conn) {
    // Init
    _style = ENGINE_STYLE_BOEING;
    _strokeSize = 2;
    _colorGround = QColor(164,147,108);
    _colorSky = QColor(0,128,255);
    _colorScale = QColor(128,128,128);
    _colorStroke = QColor(255,255,255);
    _colorWhisker = QColor(0,0,0);
    _colorValue = QColor(255,0,255);
    _colorGreen = QColor(0,255,0);
    _colorYellow = QColor(255,255,0);


    // Init attitude
    _attitude_pitchValue = 10;
    _attitude_rollValue = -20;

    // Connect

    _client.subscribeDataRef(DATAREF_PITCH,0.05);
    _client.subscribeDataRef(DATAREF_ROLL,0.05);
    _client.subscribeDataRef(DATAREF_SLIP,0.05);
    _client.subscribeDataRef(DATAREF_AIRSPEED_KTS,1.0);
    _client.subscribeDataRef(DATAREF_AIRSPEED_TARGET_KTS,1.0);
    _client.subscribeDataRef(DATAREF_ALTITUDE_FT,1.0);
    _client.subscribeDataRef(DATAREF_ALTITUDE_TARGET_FT,1.0);
    _client.subscribeDataRef(DATAREF_AIRSPEED_ACC_KTS,0.5);
    _client.subscribeDataRef(DATAREF_HEADING_DEG,1.0);
    _client.subscribeDataRef(DATAREF_VERTICALSPEED_FPM,1.0);
    _client.subscribeDataRef(DATAREF_VERTICALSPEED_TARGET,1.0);
    _client.subscribeDataRef(DATAREF_HEADING_BUG,.2);
    _client.subscribeDataRef(DATAREF_HSI_HAS_HORIZONTAL_SIGNAL,.2);
    _client.subscribeDataRef(DATAREF_HSI_HAS_VERTICAL_SIGNAL,.2);
    _client.subscribeDataRef(DATAREF_HSI_HORIZONTAL_DOTS,.05);
    _client.subscribeDataRef(DATAREF_HSI_VERTICAL_DOTS,.05);
    _client.subscribeDataRef(DATAREF_HSI_GLIDESLOPE_FLAG,.2);
    _client.subscribeDataRef(DATAREF_METRIC_PRESS,.2);
    _client.subscribeDataRef(DATAREF_BAROMETER_SETTING,.005);
    _client.subscribeDataRef(DATAREF_HSI_OBS_DEG_MAG,.2);
    _client.subscribeDataRef(DATAREF_AUTOPILOT_ON,.9);
    _client.subscribeDataRef(DATAREF_AUTOPILOT_STATE,.9);
    _client.subscribeDataRef(DATAREF_HSI_SOURCE_SELECT_PILOT,.9);
    _client.subscribeDataRef(DATAREF_NAV1_NAV_ID,1000);
    _client.subscribeDataRef(DATAREF_NAV2_NAV_ID,1000);
    _client.subscribeDataRef(DATAREF_GPS_NAV_ID,1000);
    _client.subscribeDataRef(DATAREF_HSI_HAS_DME_PILOT,.9);
    _client.subscribeDataRef(DATAREF_HSI_DME_DISTANCE_NM_PILOT,.09);
    _client.subscribeDataRef(DATAREF_HSI_DME_SPEED_KTS_PILOT,.09);




    connect(&_client, SIGNAL(refChanged(QString,QStringList)), this, SLOT(refChanged(QString,QStringList)));
    connect(&_client, SIGNAL(refChanged(QString,double)), this, SLOT(refChanged(QString,double)));
    connect(&_client, SIGNAL(refChanged(QString,QString)), this, SLOT(refChanged(QString,QString)));

}


    void PFDDisplay::refChanged(QString name, double value) {
    if (name == DATAREF_ROLL) {
        _attitude_rollValue = value;
    } else if (name == DATAREF_PITCH) {
        _attitude_pitchValue = value;
    } else if (name == DATAREF_SLIP) {
        _attitude_slipValue = value;
    } else if (name == DATAREF_AIRSPEED_KTS) {
        _airspeed_value = value;
    } else if (name == DATAREF_AIRSPEED_TARGET_KTS) {
        _airspeed_target_value = value;
    } else if (name == DATAREF_ALTITUDE_FT) {
        _altitude_value = value;
    } else if (name == DATAREF_ALTITUDE_TARGET_FT) {
        _altitude_target_value = value;
    } else if (name == DATAREF_AIRSPEED_ACC_KTS) {
        _airspeedaccl_value = value;
    } else if (name == DATAREF_HEADING_DEG) {
        _compass_heading_value =  value;
    } else if (name == DATAREF_VERTICALSPEED_FPM) {
        _verticalspeed_value =  value;
    } else if (name == DATAREF_VERTICALSPEED_TARGET) {
        _verticalspeed_target_value =  value;
    } else if (name == DATAREF_HEADING_BUG) {
        _headingBug_value =  value;
    } else if (name == DATAREF_HSI_HAS_HORIZONTAL_SIGNAL) {
        _hsi_has_horizontal_signal =  value;
    } else if (name == DATAREF_HSI_HAS_VERTICAL_SIGNAL) {
        _hsi_has_vertical_signal =  value;
    } else if (name == DATAREF_HSI_HORIZONTAL_DOTS) {
        _hsi_horizontal_dots =  value;
    } else if (name == DATAREF_HSI_VERTICAL_DOTS) {
        _hsi_vertical_dots =  value;
    } else if (name == DATAREF_HSI_GLIDESLOPE_FLAG) {
        _hsi_glideslope_flag =  value;
    } else if (name == DATAREF_METRIC_PRESS) {
        _metric_press =  value;
    } else if (name == DATAREF_BAROMETER_SETTING) {
        _barometer_setting =  value;
    } else if (name == DATAREF_HSI_OBS_DEG_MAG) {
        _hsi_obs_deg_mag =  value;
    } else if (name == DATAREF_AUTOPILOT_ON) {
        _autopilot_on =  (int)value;
    } else if (name == DATAREF_AUTOPILOT_STATE) {
        _autopilot_state =  (int)value;
    } else if (name == DATAREF_HSI_SOURCE_SELECT_PILOT) {
        _hsi_source_select_pilot =  (int)value;
    } else if (name == DATAREF_HSI_HAS_DME_PILOT) {
        _hsi_has_dme_pilot =  (int)value;
    } else if (name == DATAREF_HSI_DME_DISTANCE_NM_PILOT) {
        _hsi_dme_distance_nm_pilot =  value;
    } else if (name == DATAREF_HSI_DME_SPEED_KTS_PILOT) {
        _hsi_dme_speed_kts_pilot = (int)value;
    }



}

void PFDDisplay::refChanged(QString name, QStringList values) {

}

void PFDDisplay::refChanged(QString name, QString value) {

    if (name == DATAREF_NAV1_NAV_ID) {
        _nav1_nav_id = value;
    } else if(name == DATAREF_NAV2_NAV_ID) {
        _nav2_nav_id = value;
    } else if(name == DATAREF_GPS_NAV_ID) {
        _gps_nav_id = value;
    }

}

void PFDDisplay::itemSizeChanged(float w, float h) {
    DisplayInstrument::itemSizeChanged(w,h);
    // Sizes
    _attitude_size = qMin(w,h)*0.6;
    _scale_height = h*0.8;
    _scale_width = _attitude_size*0.22;
    _compass_size = _attitude_size*1.4;
    _verticalspeed_size = _scale_width*0.8;
    // Pens
    _defaultPen = QPen(_colorStroke,_strokeSize, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    _markerPen = QPen(_colorValue,_strokeSize, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    _defaultPen2x = QPen(_colorStroke,_strokeSize*2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    _diamondPen = QPen(_colorValue,_strokeSize*2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    // Fonts
    _tickFont = this->defaultFont;
    _tickFont.setBold(true);
    _tickFont.setPixelSize(w*0.03);

    _valueFont = this->defaultFont;
    _valueFont.setBold(true);
    _valueFont.setPixelSize(w*0.035);

    _baroFont = this->defaultFont;
    _baroFont.setBold(true);
    _baroFont.setPixelSize(w*0.03);


    _annunciatorFont = this->defaultFont;
    _annunciatorFont.setBold(true);
    _annunciatorFont.setPixelSize(w*0.025);

    // Cached pixmaps
    createCompassBackplate(_compass_size,_compass_size);
}

void PFDDisplay::render(QPainter *painter, int width, int height) {

    painter->save(); {

        // Painter init
        painter->setFont(_tickFont);

        int attitudeOffsetX = -_attitude_size*0.1;
        int annunciatorWidth = _attitude_size;
        int annunciatorHeight = (height/2) - (_attitude_size / 2);

        drawAttitudeIndicator(painter,width/2+attitudeOffsetX,height/2,_attitude_size,_attitude_size);
        drawGpsAnnunciator(painter,
                           width/2+attitudeOffsetX, // X
                           (height/2) - (_attitude_size / 2) - (annunciatorHeight / 2) + ((_attitude_size / 2)*ILS_BUFFER_SPACE), //Y
                           annunciatorWidth,
                           annunciatorHeight);

        // Draw vertical speed
        float verticalSpeedHeight = _scale_height*0.6;
        drawVerticalSpeed(painter,width,height,width-_verticalspeed_size,height/2-verticalSpeedHeight/2,_verticalspeed_size,verticalSpeedHeight);

        // Draw airspeed scale
        int scaleHeightInt = (int)_scale_height;
        if (scaleHeightInt % 2 != 0 ) scaleHeightInt++;
        drawScaleIndicator(painter,width,height,0,height/2-scaleHeightInt/2,_scale_width,scaleHeightInt,_airspeed_value,_airspeed_target_value,SCALE_INDICATOR_TYPE_AIRSPEED);

        // Draw altitude scale
        drawScaleIndicator(painter,width,height,width-_scale_width+attitudeOffsetX*2,height/2-scaleHeightInt/2,_scale_width,scaleHeightInt,_altitude_value,_altitude_target_value,SCALE_INDICATOR_TYPE_ALTITUDE);

        // Draw compass
        drawCompass(painter,width,height,width/2+attitudeOffsetX,height/2+_attitude_size*1.3,_compass_size,_compass_size);

    } painter->restore();

}

/*
    This function will draw the ILS / VOR diamond

*/
    void PFDDisplay::drawDiamond(QPainter *painter,float x,float y,float radius)
    {
        painter->setPen(_diamondPen);
        painter->setBrush(_colorValue);
        QPainterPath path;
        path.moveTo(x + radius,y);
        path.lineTo(x,y - radius );
        path.lineTo(x - radius,y);
        path.lineTo(x,y + radius );
        path.lineTo(x + radius,y);
        painter->drawPath(path);
    }

    void PFDDisplay::drawGpsAnnunciator(QPainter *painter, int annunciatorX, int annunciatorY, int annunciatorWidth, int annunciatorHeight)
    {
        painter->save();
        // Move to center position
        painter->translate(annunciatorX,annunciatorY);

        int cellWidth = annunciatorWidth / 4;
        int cellHeight = (annunciatorHeight * .8) / 4;  // Tighten the vertical spacing just a tad

        QString str;
        QRect r;

        painter->setFont(_annunciatorFont);

        // ALT MODE
        if(ALTITUDE_HOLD_ARM || ALTITUDE_HOLD_ENGAGED)
        {
            r.setRect(cellWidth,
                      -(2*cellHeight),
                      cellWidth,
                      cellHeight);
            if(ALTITUDE_HOLD_ARM)
            {
                str.sprintf("ALT ARM");
                painter->setPen(_colorYellow);
            }
            else
            {
                str.sprintf("ALT ENG");
                painter->setPen(_colorGreen);
            }
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);

        }

        // AP On/Off
        r.setRect(-cellWidth,
                  -(2*cellHeight),
                  cellWidth,
                  cellHeight);

        switch(_autopilot_on)
        {
        case 0:
            str.sprintf("AP OFF");
            painter->setPen(_colorYellow);
            break;
        case 1:
            str.sprintf("FD ON");
            painter->setPen(_colorGreen);
            break;
        case 2:
            str.sprintf("AP/FD ON");
            painter->setPen(_colorGreen);
            break;
        }
        painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);

        // HDG
        if(HEADING_HOLD_ENGAGE)
        {
            r.setRect(0,
                      -(2*cellHeight),
                      cellWidth,
                      cellHeight);
            str.sprintf("HDG SEL");
            painter->setPen(_colorGreen);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }

        // NAV
        r.setRect(0,
                  -cellHeight,
                  cellWidth,
                  cellHeight);
        if(HNAV_ARMED)
        {
            painter->setPen(_colorYellow);
            str.sprintf("LOC ARM");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        } else if(HNAV_ENGAGED)
        {
            painter->setPen(_colorGreen);
            str.sprintf("LOC ENG");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }

        // APR
        r.setRect(cellWidth,
                  -cellHeight,
                  cellWidth,
                  cellHeight);
        if(GLIDESLOPE_ARMED)
        {
            painter->setPen(_colorYellow);
            str.sprintf("G/S ARM");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        } else if(GLIDESLOPE_ENGAGED)
        {
            painter->setPen(_colorGreen);
            str.sprintf("G/S ENG");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }

        // VS
        r.setRect(cellWidth,
                  0,
                  cellWidth,
                  cellHeight);
        if(VVI_CLIMB_ENGAGED)
        {
            painter->setPen(_colorGreen);
            str.sprintf("VS ENG");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }




        // SPEED
       painter->setPen(_colorGreen);

        if(AIRSPEED_HOLD)
        {
            r.setRect(-cellWidth,
                      -cellHeight,
                      cellWidth,
                      cellHeight);

            str.sprintf("IAS");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }
        if(FLIGHT_LEVEL_CHANGE_ENGAGE)
        {
            r.setRect(-cellWidth,
                      0,
                      cellWidth,
                      cellHeight);
            str.sprintf("FLCH ENG");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }

        // ATHR
        if(AUTOTHROTTLE_ENGAGE)
        {
            r.setRect(-cellWidth,
                      cellHeight,
                      cellWidth,
                      cellHeight);
            str.sprintf("ATHR ENG");
            painter->setPen(_colorGreen);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }

        // VS
        r.setRect(cellWidth,
                  cellHeight,
                  cellWidth,
                  cellHeight);
        if(VNAV_ENGAGED)
        {
            painter->setPen(_colorGreen);
            str.sprintf("VNAV ENG");
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignHCenter,str,NULL);
        }

        painter->setPen(_defaultPen);

        // NAV Source
        r.setRect(-(2*cellWidth),
                  -(2*cellHeight),
                  cellWidth,
                  cellHeight);
        switch(_hsi_source_select_pilot)
        {
        case 0:
            str = "NAV1";
            break;
        case 1:
            str = "NAV2";
            break;
        case 2:
            str = "GPS";
            break;
        }
        painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);


        // Nav Aid Data
        r.setRect(-(2*cellWidth),
                  -(cellHeight),
                  cellWidth,
                  cellHeight);

        switch(_hsi_source_select_pilot)
        {
        case 0:
            str = _nav1_nav_id;
            break;
        case 1:
            str = _nav2_nav_id;
            break;
        case 2:
            str = _gps_nav_id;
            break;
        }
        painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);

        // DME
        if(_hsi_has_dme_pilot || _hsi_source_select_pilot == 2)     // If the VOR has DME data or we are in GPS more
        {
            r.setRect(-(2*cellWidth),
                      0,
                      cellWidth,
                      cellHeight);
            str.sprintf("%.1f NM",_hsi_dme_distance_nm_pilot);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);
            r.setRect(-(2*cellWidth),
                      cellHeight,
                      cellWidth,
                      cellHeight);
            str.sprintf("%d KTS",_hsi_dme_speed_kts_pilot);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);
        }
        else
        {
            str = "--";
            r.setRect(-(2*cellWidth),
                      0,
                      cellWidth,
                      cellHeight);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);
            r.setRect(-(2*cellWidth),
                      cellHeight,
                      cellWidth,
                      cellHeight);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);
        }

        painter->restore();

    }

void PFDDisplay::drawAttitudeIndicator(QPainter *painter, int attitudeX, int attitudeY, int attitudeWidth, int attitudeHeight) {
    painter->save(); {


        // Draw the ILS Dots
        // This assumes we have 10% of the width and height on the left and right of the attitude indicator
        // Move to center

        float IlsDotBufferSpace = ILS_BUFFER_SPACE;      // 10% of the width

        {

            painter->save();
            painter->translate(attitudeX,attitudeY);        // go to the center

            float attitudeSize = (float)attitudeWidth;
            float ilsAreaWidthandHeight = attitudeSize * (IlsDotBufferSpace / 2);
            float ilsDotRadius = ilsAreaWidthandHeight * .15;   // The dot and diamond will be 30% of the ils drawing area
            float ilsDotSpacing = (attitudeSize * .9f) / 6.0f;   // 90% of the attitude indicator for 6 dots
            float offset = (attitudeSize / 2) - (ilsAreaWidthandHeight / 2);    // Offset from the center of the attitude indicator

            painter->setPen(_defaultPen);       // White narrow pen
            for(int i = -2; i < 3;i++)  // Draw five dots
            {
                if(_hsi_has_vertical_signal && !_hsi_glideslope_flag)   // Only draw them if we have a vnav signal and glidescope capture
                {
                    painter->drawEllipse(QPointF(offset,ilsDotSpacing * i), ilsDotRadius, ilsDotRadius);
                }
                if(_hsi_has_horizontal_signal)  // Only if we have hnav signal
                {
                    painter->drawEllipse(QPointF(ilsDotSpacing * i,offset), ilsDotRadius, ilsDotRadius);
                }
            }

            float startY;       // For diamond drawing
            float startX;       // For diamond drawing
            if(_hsi_has_vertical_signal && !_hsi_glideslope_flag)
            {
                startY = _hsi_vertical_dots * ilsDotSpacing;
                startX = offset;
                drawDiamond(painter,startX,startY,ilsDotRadius);
            }
            if(_hsi_has_horizontal_signal)
            {
                startX = _hsi_horizontal_dots * ilsDotSpacing;
                startY = offset;
                drawDiamond(painter,startX,startY,ilsDotRadius);
            }

            QString str;
            painter->setPen(_colorStroke);
            painter->setFont(_baroFont);


            // CRS
            QRect r(-(attitudeWidth/2),
                    (attitudeWidth/2) + (ilsAreaWidthandHeight),
                    attitudeWidth/2,
                    ilsAreaWidthandHeight);

            str.sprintf("CRS %03d",(int)_hsi_obs_deg_mag);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);

            // HDG
            r.setRect(0,
                      (attitudeWidth/2) + (ilsAreaWidthandHeight),
                      attitudeWidth/2,
                      ilsAreaWidthandHeight);
            str.sprintf("HDG %03d",(int)_headingBug_value);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignRight,str,NULL);

            painter->restore();
        }



        // The attidude indicator takes up 90% of the drawing space
        attitudeWidth = attitudeWidth * (1.0f-IlsDotBufferSpace);
        attitudeHeight = attitudeHeight * (1.0f-IlsDotBufferSpace);

        // Init
        double pitch = _attitude_pitchValue;
        double pixelsPerPitchDeg = (attitudeWidth*0.02f);
        //pitch = 10;
        //if (pitch > maxPitch) pitch = maxPitch;
        //if (pitch < (0-maxPitch)) pitch = (0-maxPitch);

        // Move to center
        painter->translate(attitudeX,attitudeY);

        // Make sure to clip the widget because of moving card
        QPainterPath clipPath;
        clipPath.setFillRule(Qt::WindingFill);
        clipPath.addRoundedRect(QRect(-attitudeWidth/2,-attitudeHeight/2,attitudeWidth,attitudeHeight), attitudeWidth*0.1, attitudeHeight*0.1 );
        painter->setClipPath(clipPath);

        // Draw background (sky)
        painter->fillRect(-attitudeWidth/2,-attitudeHeight/2,attitudeWidth,attitudeHeight,_colorSky);

        // Rotate to roll and draw moving card
        //TODO: cache this as a pixmap?
        painter->save(); {
            double pitchPixelsY = pitch*pixelsPerPitchDeg;
            //DEBUG << pitch;
            painter->rotate(-_attitude_rollValue);
            painter->translate(0,pitchPixelsY);

            // Draw ground
            int groundWidth = attitudeWidth*2;
            int groundHeight = attitudeHeight*2;
            painter->fillRect(-groundWidth/2,0,groundWidth,groundHeight,_colorGround);

            // Draw ground line
            painter->fillRect(-groundWidth/2,-1,groundWidth,2,_colorStroke);

            // Draw pitch markers
            int degreesPerMarker = 10;
            for(int p = -90; p < 90; p += degreesPerMarker) {
                // Init
                int markerWidth = attitudeWidth*0.3;
                int middleMarkerWidth = markerWidth*0.4;
                int smallMarkerWidth = middleMarkerWidth*0.4;
                int markerHeight = _strokeSize;
                int pOffset = -p*pixelsPerPitchDeg;
                int markerY = -markerHeight/2+pOffset;
                // Main Marker
                painter->fillRect(-markerWidth/2,
                                  markerY,
                                  markerWidth,
                                  markerHeight,
                                  _colorStroke);
                // Middle Marker
                painter->fillRect(-middleMarkerWidth/2,
                                  markerY + (degreesPerMarker/2.0)*pixelsPerPitchDeg,
                                  middleMarkerWidth,
                                  markerHeight,
                                  _colorStroke);
                // Small Marker
                painter->fillRect(-smallMarkerWidth/2,
                                  markerY + (degreesPerMarker/4.0)*pixelsPerPitchDeg,
                                  smallMarkerWidth,
                                  markerHeight,
                                  _colorStroke);
                painter->fillRect(-smallMarkerWidth/2,
                                  markerY + (degreesPerMarker*0.75)*pixelsPerPitchDeg,
                                  smallMarkerWidth,
                                  markerHeight,
                                  _colorStroke);
                // Text
                int textWidth = markerWidth*0.3;
                int textHeight = textWidth/2;
                if(p % 10 == 0 && p != 0) {
                    painter->setPen(_defaultPen);
                    //QRect r1(markerWidth/2,-textHeight/2+pOffset,markerWidth+textWidth,textHeight);
                    QRect r(-markerWidth/2-textWidth,-textHeight/2+pOffset,markerWidth+textWidth+textWidth,textHeight);
                    painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,QString("%1").arg(abs(p)),NULL);
                    painter->drawText(r,Qt::AlignVCenter|Qt::AlignRight,QString("%1").arg(abs(p)),NULL);
                }
            }

        } painter->restore();

        // Draw static attitude markers for roll
        int tickRadius = attitudeHeight/2;
        int tickSize = attitudeHeight*0.08;
        int triangleSize = tickSize/2;
        {
            // Line markings
            painter->setPen(_defaultPen);
            double r = tickRadius;
            double s = tickSize;
            double l1 = r-s;
            double l2l = r;
            double l2s = r-(s/2);
            // Short inner
            for(int i=70; i<=110; i+=10){
                if(i == 90) continue;
                painter->drawLine(l1*cos(float(i)/180.*M_PI),-l1*sin(float(i)/180.*M_PI),
                                  l2s*cos(float(i)/180.*M_PI),-l2s*sin(float(i)/180.*M_PI));
            }
            // Short outer at 45deg
            {
                int i=45;
                painter->drawLine(l1*cos(float(i)/180.*M_PI),-l1*sin(float(i)/180.*M_PI),
                                  l2s*cos(float(i)/180.*M_PI),-l2s*sin(float(i)/180.*M_PI));
            }
            {
                int i=90+45;
                painter->drawLine(l1*cos(float(i)/180.*M_PI),-l1*sin(float(i)/180.*M_PI),
                                  l2s*cos(float(i)/180.*M_PI),-l2s*sin(float(i)/180.*M_PI));
            }
            // Long
            for(int i=30; i<=180-30; i+=30){
                if(i == 90) continue;
                painter->drawLine(l1*cos(float(i)/180.*M_PI),-l1*sin(float(i)/180.*M_PI),
                                  l2l*cos(float(i)/180.*M_PI),-l2l*sin(float(i)/180.*M_PI));
            }
            // Fixed triangle at 0deg
            QPainterPath path;
            path.moveTo(-triangleSize,-(r-triangleSize));
            path.lineTo(+triangleSize,-(r-triangleSize));
            path.lineTo(0,-(r-triangleSize*2));
            painter->fillPath(path,_colorStroke);
        }

        // Draw roll marker
        {
            // Rotate
            painter->rotate(-_attitude_rollValue);
            // Fixed triangle at 0deg
            QPainterPath path;
            int r = tickRadius-triangleSize;
            int rollMarkerSize = triangleSize*1.3;
            path.moveTo(-rollMarkerSize,-(r-rollMarkerSize*2));
            path.lineTo(+rollMarkerSize,-(r-rollMarkerSize*2));
            path.lineTo(0,-(r-rollMarkerSize));
            painter->fillPath(path,_colorStroke);
            // Un-Rotate
            painter->rotate(+_attitude_rollValue);
            // Rotate
            painter->rotate(-_attitude_slipValue);
            painter->fillRect(QRect(-rollMarkerSize,-r+rollMarkerSize*2+_strokeSize,rollMarkerSize*2,_strokeSize),_colorStroke);
            painter->rotate(+_attitude_slipValue);
        }

        // Draw wiskers
        painter->save(); {
            // Init
            painter->setBrush(_colorWhisker);
            painter->setPen(QPen(_colorStroke,_strokeSize/2));
            int w = attitudeWidth*0.3;
            int h = w*0.1;
            int pad = w*0.18;
            QPainterPath path;
            path.moveTo(0,0);
            path.lineTo(w,0);
            path.lineTo(w,h*2);
            path.lineTo(w-h,h*2);
            path.lineTo(w-h,h);
            path.lineTo(0,h);
            path.lineTo(0,0);
            // Left
            painter->translate(-(w+pad),-h/2);
            painter->drawPath(path);
            // Right
            painter->scale(-1,1);
            painter->translate(-(w+pad)*2,0);
            painter->drawPath(path);
        } painter->restore();


    } painter->restore();
}

int roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = abs(numToRound) % multiple;
    if (remainder == 0)
        return numToRound;

    if (numToRound < 0)
        return -(abs(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

void PFDDisplay::drawScaleIndicator(QPainter *painter, int screenWidth, int screenHeight, int x, int y, int w, int h, double value, double target_value,int type) {
    painter->save(); {
        // Init
        int intValue = (int)value;
        int intTargetValue = (int)target_value;
        QRect mainRect = QRect(0,0,w,h);

        // Setup for type (default = airspeed)
        double ticks = 13;
        double valuePerTick = 10;
        int textEveryXValue = 20;
        if(type == SCALE_INDICATOR_TYPE_ALTITUDE) {
            ticks = 9;
            valuePerTick = 100;
            textEveryXValue = 200;
        }
        double pixelsPerTick = h/ticks;
        double pixelsPerValue = h/(ticks*valuePerTick);

        // Move to position
        painter->translate(x,y);

        // Draw background
        painter->fillRect(mainRect,Qt::gray);

        // Move to center
        painter->translate(0,h/2);

        // Get tick offset
        // We need both a int version and a back-calculated float version
        // The int is used for drawing labels, the float version for drawing
        int valueOffset = intValue % (int)valuePerTick;
        double valueOffsetFloat = valueOffset + value - intValue;
        int pixelOffset = valueOffset * pixelsPerValue;
        double pixelOffsetFloat = valueOffsetFloat * pixelsPerValue;

        // Draw ticks
        painter->setClipRect(QRect(0,-h/2,w,h));
        painter->setPen(_defaultPen);
        int tickWidth = w*0.16;
        for(int t = -ticks/2-1; t < ticks/2+1; t++) {
            // Get value and pixel
            // Here we need
            int tv = (value-valueOffset) + t*valuePerTick*-1;
            tv = roundUp(tv+1,valuePerTick)-valuePerTick;
            int tpy = t*pixelsPerTick+pixelOffsetFloat;
            // Draw tick
            if(type == SCALE_INDICATOR_TYPE_ALTITUDE) {
                painter->fillRect(0,tpy,tickWidth,_strokeSize,_colorStroke);
            } else {
                painter->fillRect(w-tickWidth,tpy,tickWidth,_strokeSize,_colorStroke);
            }
            // Draw text
            if(tv % textEveryXValue==0) {
                if(type == SCALE_INDICATOR_TYPE_ALTITUDE) {
                    int textWidth = w;
                    int textHeight = w;
                    painter->drawText(QRect(0,tpy-textHeight/2,textWidth*0.95,textHeight),
                                      Qt::AlignVCenter|Qt::AlignRight,
                                      QString("%1").arg(tv),NULL);
                } else {
                    int textWidth = w-tickWidth-tickWidth*0.2;
                    int textHeight = w;
                    painter->drawText(QRect(0,tpy-textHeight/2,textWidth,textHeight),
                                      Qt::AlignVCenter|Qt::AlignRight,
                                      QString("%1").arg(tv),NULL);
                }
            }
        }

        // Draw value box
        {
            int boxHeight = w *.6;  // 60% of the tape width
            float pointerBodyFactor = .8f;       // The pointer body is 80% of the width
            float pointerFactor = 1.0f - pointerBodyFactor;  // The pointer is what is left over
            painter->save();
            QPainterPath path;
            if(type == SCALE_INDICATOR_TYPE_ALTITUDE)
            {
                path.moveTo(pointerFactor * w,-.5 * boxHeight);    // Left - Lower
                path.lineTo(w,-.5 * boxHeight);         // Right - Lower
                path.lineTo(w,.5 * boxHeight);          // Upper - Right
                path.lineTo(pointerFactor * w,.5 * boxHeight);     // Upper - Left
                path.lineTo(0,0);                       // Middle - Left
                path.lineTo(pointerFactor * w,-.5 * boxHeight);    // Left - Lower
            }
            else
            {
                path.moveTo(0,-.5 * boxHeight);         // Left - Lower
                path.lineTo(pointerBodyFactor * w,-.5 * boxHeight);    // Right - Lower
                path.lineTo(1 * w,0);                   // Middle - Right
                path.lineTo(pointerBodyFactor * w,.5 * boxHeight);     // Upper - Right
                path.lineTo(0,.5 * boxHeight);          // Upper - Left
                path.lineTo(0,-.5 * boxHeight);         // Left - Lower
            }
            painter->setPen(_defaultPen);
            painter->setBrush(Qt::black);
            painter->drawPath(path);

            // Draw text
            if(type == SCALE_INDICATOR_TYPE_ALTITUDE)
            {
                QRect r(w * pointerFactor,-(.5 * boxHeight),w * pointerBodyFactor,boxHeight);
                painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,QString("%1").arg(intValue),NULL);
            }
            else
            {
                QRect r(0,-(.5 * boxHeight),w * pointerBodyFactor,boxHeight);
                painter->drawText(r,Qt::AlignVCenter|Qt::AlignRight,QString("%1").arg(intValue),NULL);

            }

            // Draw Target Markers


            float startingY = (intValue - intTargetValue) * pixelsPerValue;
            QPainterPath markerPath;
            if(type == SCALE_INDICATOR_TYPE_ALTITUDE)
            {
                markerPath.moveTo(0,startingY);
                markerPath.lineTo(w * pointerFactor,startingY - (.5 * boxHeight));
                markerPath.lineTo(0,startingY - (.5 * boxHeight));
                markerPath.lineTo(0,startingY + (.5 * boxHeight));
                markerPath.lineTo(w * pointerFactor,startingY + (.5 * boxHeight));
                markerPath.lineTo(0,startingY);
            }
            else
            {
                markerPath.moveTo(w,startingY);
                markerPath.lineTo(pointerBodyFactor * w,startingY - (.5 * boxHeight));
                markerPath.lineTo(w,startingY - (.5 * boxHeight));
                markerPath.lineTo(w,startingY + (.5 * boxHeight));
                markerPath.lineTo(pointerBodyFactor * w,startingY + (.5 * boxHeight));
                markerPath.lineTo(w,startingY);
            }

            painter->setPen(_markerPen);
            painter->setBrush(Qt::transparent);
            painter->drawPath(markerPath);

            painter->restore();
        }


        painter->setClipping(false);
        // Draw target value
        {
            int valueWidth = w;
            int valueHeight = valueWidth*0.3;
            QRect r(0,-h/2-valueHeight*1.3,valueWidth,valueHeight);
            painter->setPen(_colorValue);
            painter->setFont(_valueFont);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignCenter,QString("%1").arg(intTargetValue),NULL);
        }

        // Draw target value
        if(type == SCALE_INDICATOR_TYPE_ALTITUDE)
        {
            int valueWidth = w;
            int valueHeight = valueWidth*0.2;
            QString str;
            QRect r(0,h/2+valueHeight,valueWidth * 2,valueHeight);
            painter->setPen(_colorStroke);
            painter->setFont(_baroFont);
            if(_metric_press)
            {
                str =QString("%1 HPA").arg(_barometer_setting * MILLIBARS_FACTOR,4,'f',0);
            }
            else
            {
                str =QString("%1 IN").arg(_barometer_setting,5,'f',2);
            }
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,str,NULL);
        }

    }
    painter->restore();
}

void PFDDisplay::createCompassBackplate(int w, int h) {

    // Cached version - improves performance and allow smoother rending of angled texts

    // Create image
    QImage image = QImage(QSize(w,h), QImage::Format_ARGB32);
    image.fill(0x00ffffff);

    // Init painter
    QPainter painter;
    painter.begin(&image);
    setupPainter(&painter);
    painter.setFont(_tickFont);

    // Translate to center
    painter.translate(w/2,h/2);

    // Draw backplate
    painter.setPen(Qt::transparent);
    painter.setBrush(_colorScale);
    painter.drawEllipse(QPoint(0,0),w/2,w/2);

    // Rotate to heading
    //painter->rotate(_compass_heading_value);

    // Draw ticks
    painter.setPen(_defaultPen);
    int degreesPerTick = 5;
    float r = w/2.0;
    float rtL = r - r*0.1; // Long tick
    float rtS = r - r*0.05; // Short tick
    float textWidth = w*0.1;
    for(int i=0; i<360; i+=degreesPerTick){
        // Rotate
        painter.rotate(i);
        float rt = rtS;
        // Text
        if(i % (degreesPerTick*2) == 0) {
            rt = rtL;
            int displayValue = i / 10;
            QRect r(-textWidth/2,-rtL,textWidth,textWidth/2);
            painter.drawText(r,Qt::AlignVCenter|Qt::AlignCenter,QString("%1").arg(displayValue),NULL);
        }
        // Tick
        painter.drawLine(0,rt,0,r);
        // Undo Rotate
        painter.rotate(-i);
    }


    // Save as pixmap
    painter.end();
    _compass_backplate = QPixmap::fromImage(image, Qt::AutoColor);


}

void PFDDisplay::drawCompass(QPainter *painter, int screenWidth, int screenHeight, int x, int y, int w, int h) {
    painter->save(); {

        // Move to center and rotate
        painter->translate(x,y);
        painter->rotate(-_compass_heading_value);

        // Draw backplate
        painter->drawPixmap(-_compass_size/2,-_compass_size/2,_compass_backplate);

        // Undo rotate
        painter->rotate(+_compass_heading_value);

        // Draw center line
        float r = w/2.0;
        painter->setPen(QPen(_colorStroke,_strokeSize*1.4,Qt::DotLine));
        painter->drawLine(0,0,0,-r);

        // Draw triangle at top
        float triangleSize = _compass_size*0.02;
        QPainterPath path;
        path.moveTo(0,-r);
        path.lineTo(-triangleSize,-r-triangleSize*1.6);
        path.lineTo(+triangleSize,-r-triangleSize*1.6);
        path.lineTo(0,-r);
        painter->setPen(_defaultPen);
        painter->drawPath(path);

        // Draw heading bug
        {
            painter->save();
            float bugSize = _compass_size*0.04;
            float bugCellSize = bugSize / 3;    //This will be a 3 X 3 graphic

            r = r - bugSize;    // Place the bug right on the card
            QPainterPath bugPath;
            bugPath.moveTo(0,-r);
            bugPath.lineTo(-bugCellSize,-r-bugCellSize);
            bugPath.lineTo(-(2*bugCellSize),-r-bugCellSize);
            bugPath.lineTo(-(2*bugCellSize),-r-(3*bugCellSize));
            bugPath.lineTo((2*bugCellSize),-r-(3*bugCellSize));
            bugPath.lineTo((2*bugCellSize),-r-(1*bugCellSize));
            bugPath.lineTo((bugCellSize),-r-(1*bugCellSize));
            bugPath.lineTo(0,-r);

            painter->setPen(_markerPen);
            painter->rotate(-_compass_heading_value);
            painter->rotate(_headingBug_value);
            painter->drawPath(bugPath);
            painter->restore();
        }

        // Draw value
        {
            painter->translate(-x,-y);
            int intValue = (int)_compass_heading_value;
            int valueWidth = h*0.18;
            int valueHeight = valueWidth*0.3;
            QRect r(x-valueWidth,screenHeight-valueHeight*1.2,valueWidth,valueHeight);
            painter->setPen(_colorValue);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignCenter,QString("%1H").arg(intValue),NULL);
        }



    } painter->restore();
}

void PFDDisplay::drawVerticalSpeed(QPainter *painter, int screenWidth, int screenHeight, int x, int y, int w, int h) {
    painter->save(); {

        // Vertical speed is in feet per minute, usually every tick @ 500
        //TODO: Implement that top and bottom tick on a different scale
        //TODO: Use a simulated needle instead (like on older boings)

        // Init
        int intValue = (int)_verticalspeed_value;
        int intTargetValue = (int)_verticalspeed_target_value;
        double ticks = 6;
        int valuePerTick = 500;
        int textEveryXValue = 1000;
        int tickValueDivisor = 1000;
        double pixelsPerTick = h/(ticks*2);
        double pixelsPerValue = h/((ticks*2)*valuePerTick);
        double tickWidth = w*0.4;
        double textWidth = w-tickWidth;
        double textHeight = textWidth/2;

        // Move to position
        painter->translate(x,y);

        // Draw background
        int padding = textHeight;
        QRect mainRect = QRect(0,0-padding,w,h+padding+padding);
        painter->fillRect(mainRect,Qt::gray);

        // Move to center
        painter->translate(0,h/2);

        // Draw ticks
        painter->setPen(_defaultPen);
        for(int t = -ticks; t <= ticks; t++) {
            // Draw text and tick
            double tickY = t * pixelsPerTick;
            double tickS = tickWidth * 0.5;
            int tickValue = abs(t * valuePerTick);
            int displayValue = tickValue / tickValueDivisor;
            if(tickValue % textEveryXValue == 0) {
                tickS = tickWidth;
                QRect r(tickWidth*1.1,tickY-textHeight/2,textWidth,textHeight);
                if(displayValue != 0) painter->drawText(r,Qt::AlignVCenter|Qt::AlignLeft,QString("%1").arg(displayValue),NULL);
            }
            painter->drawLine(0,tickY,tickS,tickY);
        }

        // Draw value line
        double valueY = _verticalspeed_value * pixelsPerValue * -1;
        if(valueY < -h/2) valueY = -h/2;
        else if(valueY > h/2) valueY = h/2;
        painter->setPen(_defaultPen2x);
        painter->drawLine(w/2,valueY,w,valueY);

        // Draw target value
        {
            int valueWidth = w;
            int valueHeight = w;
            QRect r(0,-((h/2) + valueHeight),valueWidth,valueHeight);
            painter->setPen(_colorValue);
            QFont saveFont = painter->font();
            painter->setFont(_valueFont);
            painter->drawText(r,Qt::AlignVCenter|Qt::AlignCenter,QString("%1").arg(intTargetValue),NULL);
            painter->setFont(saveFont);
        }


    } painter->restore();
}


void PFDDisplay::storeSettings(QSettings &settings) {
    DisplayInstrument::storeSettings(settings);

    settings.setValue("style", _style);


}

void PFDDisplay::loadSettings(QSettings &settings) {
    DisplayInstrument::loadSettings(settings);

    setStyle(settings.value("style","0").toInt());

}

void PFDDisplay::createSettings(QGridLayout *layout) {
    DisplayInstrument::createSettings(layout);

    QStringList styles;
    styles << "Generic" << "Boing";
    createComboBoxSetting(layout,"Style",_style,styles,SLOT(setStyle(int)));
    //createSliderSetting(layout,"Number of Labels",0,11,_barLabels,SLOT(setBarLabels(int)));
    //createCheckboxSetting(layout,"Auto Min",_autoMin,SLOT(setAutoMin(bool)));
    //createCheckboxSetting(layout,"Auto Max",_autoMax,SLOT(setAutoMax(bool)));


}

