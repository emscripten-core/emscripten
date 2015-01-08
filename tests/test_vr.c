#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/vr.h>

static int gDevCount = -1;
static int gPosDev = -1;
static int gHmdDev = -1;

void
report_result(int result)
{
    emscripten_cancel_main_loop();
    if (result == 0) {
        printf("Test successful!\n");
    } else {
        printf("Test failed!\n");
    }
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    exit(result);
}

static void
mainloop()
{
    static int loopcount = 0;

    if (!emscripten_vr_ready()) {
        printf("VR not ready\n");
        return;
    }

    if (gDevCount == -1) {
        gDevCount = emscripten_vr_count_devices();
        if (gDevCount == 0) {
            printf("No VR devices found!\n");
            report_result(0);
            return;
        }

        int hwid = -1;
        for (int i = 0; i < gDevCount; ++i) {
            WebVRDeviceId devid = emscripten_vr_get_device_id(i);
            if (hwid == -1 || hwid == emscripten_vr_get_device_hwid(devid)) {
                hwid = emscripten_vr_get_device_hwid(devid);

                if (emscripten_vr_get_device_type(devid) == WebVRHMDDevice) {
                    gHmdDev = devid;
                } else if (emscripten_vr_get_device_type(devid) == WebVRPositionSensorDevice) {
                    gPosDev = devid;
                }
            }
        }

        if (gHmdDev == -1 || gPosDev == -1) {
            printf("Couln't find both a HMD and position device\n");
            // this is a failure because it's weird
            report_result(1);
            return;
        }

        WebVRFieldOfView leftFov, rightFov;
        emscripten_vr_hmd_get_current_fov(gHmdDev, WebVREyeLeft, &leftFov);
        emscripten_vr_hmd_get_current_fov(gHmdDev, WebVREyeRight, &rightFov);

        printf("Left FOV: %f %f %f %f\n", leftFov.upDegrees, leftFov.downDegrees, leftFov.rightDegrees, leftFov.leftDegrees);
        printf("Right FOV: %f %f %f %f\n", rightFov.upDegrees, rightFov.downDegrees, rightFov.rightDegrees, rightFov.leftDegrees);
    }

    WebVRPositionState state;
    emscripten_vr_sensor_get_state(gPosDev, 0.0, &state);
    printf("State: orientation: [%f %f %f %f] position: [%f %f %f]\n",
           state.orientation.x, state.orientation.y, state.orientation.z, state.orientation.w,
           state.position.x, state.position.y, state.position.z);

    if (loopcount++ > 10) {
        report_result(0);
    }
}

int
main(int argc, char **argv)
{
    emscripten_vr_init();

    /* 2fps -- no rAF */
    emscripten_set_main_loop(mainloop, 2, 0);
}
