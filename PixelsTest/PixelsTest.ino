//#include <Pixels_PPI16.h>
//#include <Pixels_Antialiasing.h>  // optional (a removal does not impact fonts antialiasing)
//#include <Pixels_HX8352.h>


#include <Pixels_Antialiasing.h>
#include <Pixels_SPIsw.h>
#include <Pixels_ILI9341.h>

//Pixels pxs(240, 400);

Pixels pxs(240, 320);



void setup() {
		pxs.init();
		pxs.clear();
		pxs.setBackground(100, 100, 100);
	  pxs.setColor(100, 100, 100);
    pxs.drawPixel(10, 10);
	}

 void loop() {
  
  }
