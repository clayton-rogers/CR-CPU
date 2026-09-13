#include "cmdline_parser.h"
#include "compiler.h"


int main(int argc, char** argv)
{
	auto opt = parse_args(argc, argv);
	return compile(opt);
}
