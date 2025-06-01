/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void screen_harmonic_event_handler (lv_event_t *e)
{

        lv_event_code_t code = lv_event_get_code(e); //获取产生事件对象的事件码
        lv_obj_t* chart = lv_event_get_target(e); //获取产生事件的对象
        if (code == LV_EVENT_VALUE_CHANGED) { //值已改变事件
            lv_obj_invalidate(chart);  // 刷新CHART显示
        }
        if (code == LV_EVENT_REFR_EXT_DRAW_SIZE) { //LV_EVENT_REFR_EXT_DRAW_SIZE事件
            lv_coord_t* s = lv_event_get_param(e); // 获取事件对象的size数据
            *s = LV_MAX(*s, 20);    //得到最大值,且最大值限制在20以上
        }
        else if (code == LV_EVENT_DRAW_POST_END) { // LV_EVENT_DRAW_POST_END事件
            int32_t id = lv_chart_get_pressed_point(chart); // 	获取CHART上被点击的点ID
            if (id == LV_CHART_POINT_NONE) return; // 不是CHART上的点，则不处理，直接返回
            LV_LOG_USER("Selected point %d", (int)id); //打印点ID值
            lv_chart_series_t* ser = lv_chart_get_series_next(chart, NULL); // 获取第一条ser线
            while (ser) {
                lv_point_t p;
                lv_chart_get_point_pos_by_id(chart, ser, id, &p); // 通过CHART的点ID获取其在CHART上的位置值并保存在p中
                lv_coord_t* y_array = lv_chart_get_y_array(chart, ser); // 获取CHART的ser对应点值的数组集合
                lv_coord_t value = y_array[id]; //获取ID对应的ser的点数值
                char buf[16];
                lv_snprintf(buf, sizeof(buf), LV_SYMBOL_DUMMY"%d%%", value); //ser的点值转成字串到buf
                lv_draw_rect_dsc_t draw_rect_dsc;
                lv_draw_rect_dsc_init(&draw_rect_dsc);
                draw_rect_dsc.bg_color = lv_color_black();  //黑色背景
                draw_rect_dsc.bg_opa = LV_OPA_70;  // 50%透明度
                draw_rect_dsc.radius = 3;    // 圆角半径为3
                draw_rect_dsc.bg_img_src = buf; // 显示数据指向buf
                draw_rect_dsc.bg_img_recolor = lv_color_white(); //重新着色为白色
                lv_area_t a;
                a.x1 = chart->coords.x1 + p.x - 20;
                a.x2 = chart->coords.x1 + p.x + 20;
                a.y1 = chart->coords.y1 + p.y - 30;
                a.y2 = chart->coords.y1 + p.y - 10;
                lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e); //获取事件对象的绘制上下文对象
                lv_draw_rect(draw_ctx, &draw_rect_dsc, &a); //绘制点数值到a矩形区
                ser = lv_chart_get_series_next(chart, ser); //获取下一条ser线
            }
        }
        else if (code == LV_EVENT_RELEASED) { //点击松手事件
            lv_obj_invalidate(chart); // 刷新CHART显示
        }
}

void events_init_screen (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_harmonic, screen_harmonic_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
