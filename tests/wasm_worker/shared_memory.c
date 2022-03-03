#include <emscripten.h>

int main()
{
	_ReportResult(EM_ASM_INT(return buffer instanceof SharedArrayBuffer), 0);
}
