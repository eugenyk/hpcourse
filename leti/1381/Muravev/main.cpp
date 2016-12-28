#include "settingsGraph.h"
#include "addFuction.h"
#include "pixel.h"
#include "base.h"
#include "image.h"
#include "flowGraph.h"
 int main(int argc, char *argv[])
 {
	 base args;
	 if (!parse(argc, argv, args))
		 return -1;

	 flowGraph(args);

	 return 0;
 }