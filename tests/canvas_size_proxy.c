#include <math.h>
#include <emscripten/html5.h>

int main() 
{
    int result = 0;
    double w, h;
    emscripten_get_element_css_size(NULL, &w, &h);
    if (isnan(w) || isnan(h))
    {
        result = 1;
    }
    REPORT_RESULT();
    return 0;
}
