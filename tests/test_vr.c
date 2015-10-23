#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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
        printf("%d VR devices found\n", gDevCount);


        int hwid = -1;
        char *devName;
        for (int i = 0; i < gDevCount; ++i) {
            WebVRDeviceId devid = emscripten_vr_get_device_id(i);
            if (hwid == -1 || hwid == emscripten_vr_get_device_hwid(devid)) {
                hwid = emscripten_vr_get_device_hwid(devid);
                devName = emscripten_vr_get_device_name(devid);

                if (emscripten_vr_get_device_type(devid) == WebVRHMDDevice) {
                    gHmdDev = devid;
                    printf("Using WebVRHMDDevice '%s' (deviceId '%d'; hardwareUnitId '%d'.)\n", devName, gHmdDev,  hwid);
                } else if (emscripten_vr_get_device_type(devid) == WebVRPositionSensorDevice) {
                    gPosDev = devid;
                    printf("Using WebVRPositionSensorDevice '%s' (deviceId '%d'; hardwareUnitId '%d').\n", devName, gPosDev,  hwid);
                }
            }
        }

        if (gHmdDev == -1 || gPosDev == -1) {
            printf("Couln't find both a HMD and position device\n");
            // this is a failure because it's weird
            report_result(1);
            return;
        }

        WebVREyeParameters leftParams, rightParams;
        emscripten_vr_hmd_get_eye_parameters(gHmdDev, WebVREyeLeft, &leftParams);
        emscripten_vr_hmd_get_eye_parameters(gHmdDev, WebVREyeLeft, &rightParams);

        WebVRFieldOfView leftFov = leftParams.currentFieldOfView, rightFov = rightParams.currentFieldOfView;
        printf("Left FOV: %f %f %f %f\n", leftFov.upDegrees, leftFov.downDegrees, leftFov.rightDegrees, leftFov.leftDegrees);
        printf("Right FOV: %f %f %f %f\n", rightFov.upDegrees, rightFov.downDegrees, rightFov.rightDegrees, rightFov.leftDegrees);
    }

    WebVRPositionState state;
    emscripten_vr_sensor_get_state(gPosDev, false, &state);
    printf("Timestamp: %f, hasPosition: %s , hasOrientation: %s\n", state.timeStamp, 
           state.hasPosition ? "true" : "false", state.hasOrientation ? "true" : "false");
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
