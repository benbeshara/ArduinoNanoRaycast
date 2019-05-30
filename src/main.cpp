#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// Touch TFT Setup
#define TFT_DC 9
#define TFT_CS 10

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define STMPE_CS 8

Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Raycasting Setup
struct Vector2
{
  double x;
  double y;
};

Vector2 mapSize = {
  24,
  24
};

int world[24][24] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

Vector2 screenDimensions = {
  320,
  240
};

Vector2 playerPos = {
  12,
  19
};

Vector2 playerDir = {
  -1,
  0
};

Vector2 cameraPlane = {
  0,
  0.75
};

double moveSpeed = 0.2;
double rotSpeed = 0.1;

float ft = 0;
float lastFt = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  screen.begin();
  screen.setRotation(1);

  Serial.println("Initialised Screen...");
  
  if(!ts.begin()){
    Serial.println("Touch Screen initialisation failed!");
  } else {
    Serial.println("Initialised Touchscreen...");
  }

  int8_t x = screen.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = screen.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = screen.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = screen.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = screen.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  Serial.println(F("Ready!"));
}

void loop() {
  bool forward = false;
  bool backward = false;
  bool left = false;
  bool right = false;
  
  // put your main code here, to run repeatedly:
  if(!ts.bufferEmpty() || ts.touched()){
    TS_Point tp = ts.getPoint();

    tp.x = map(tp.x, TS_MINX, TS_MAXX, 0, screen.width());
    tp.y = map(tp.y, TS_MINY, TS_MAXY, 0, screen.height());

    if(tp.x < 30){
      backward = true;
    }
    if(tp.x > 210){
      forward = true;
    }
    if(tp.y < 30){
      left = true;
    }
    if(tp.y > 290){
      right = true;
    }
  }

  int32_t startTime = millis();
  for(int16_t x = 1; x < screenDimensions.x; x++){
    double cameraX = 2 * x / double(screenDimensions.x) - 1;
    
    Vector2 rayDir = {
      playerDir.x + cameraPlane.x * cameraX,
      playerDir.y + cameraPlane.y * cameraX
    };

    Vector2 mapLoc = {
      int(playerPos.x),
      int(playerPos.y)
    };

    Vector2 sideDist;

    Vector2 deltaDist = {
      abs(1 / rayDir.x),
      abs(1 / rayDir.y)
    };

    double perpWallDist;

    Vector2 step;

    int side;

    if(rayDir.x < 0){
      step.x = -1;
      sideDist.x = (playerPos.x - mapLoc.x) * deltaDist.x;
    } else {
      step.x = 1;
      sideDist.x = (mapLoc.x + 1.0 - playerPos.x) * deltaDist.x;
    }
    
    if(rayDir.y < 0){
      step.y = -1;
      sideDist.y = (playerPos.y - mapLoc.y) * deltaDist.y;
    } else {
      step.y = 1;
      sideDist.y = (mapLoc.y + 1.0 - playerPos.y) * deltaDist.y;
    }

    while(true){
      if(sideDist.x < sideDist.y){
        sideDist.x += deltaDist.x;
        mapLoc.x += step.x;
        side = 0;
      } else {
        sideDist.y += deltaDist.y;
        mapLoc.y = step.y;
        side = 1;
      }
      if(world[(int)mapLoc.x][(int)mapLoc.y] > 0) break;
    }

    if(side < 1){
      perpWallDist = (mapLoc.x - playerPos.x + (1 - step.x) / 2) / rayDir.x;
    } else {
      perpWallDist = (mapLoc.y - playerPos.y + (1 - step.y) / 2) / rayDir.y;
    }
  
    int16_t wallHeight = (int)(screenDimensions.y / perpWallDist);
    int16_t wallTop = -wallHeight / 2 + screenDimensions.y / 2;
    if(wallTop < 0){
      wallTop = 0;
    }
    int16_t wallEnd = wallHeight / 2 + screenDimensions.y / 2;
    if(wallEnd < 0){
      wallEnd = 0;
    }
    
    int16_t negativeSpaceHeight = (screenDimensions.y - wallHeight) / 2;
    int16_t colour;

    switch(world[(int)mapLoc.x][(int)mapLoc.y]){
      case 1:
        colour = ILI9341_BLUE;
        break;

      case 2:
        colour = ILI9341_RED;
        break;

      case 3:
        colour = ILI9341_GREEN;
        break;

      case 4:
        colour = ILI9341_YELLOW;
        break;

      case 5:
        colour = ILI9341_PURPLE;
        break;

      default:
        colour = ILI9341_WHITE;
    }

    if(side > 0){
      colour = colour / 2;
    };

    screen.drawFastVLine(x, 0, negativeSpaceHeight, ILI9341_LIGHTGREY);
    screen.drawFastVLine(x, wallEnd, negativeSpaceHeight, ILI9341_DARKGREY);
    screen.drawFastVLine(x, wallTop, wallHeight, colour);
  }

  lastFt = ft;
  ft = millis() - startTime;
  double fps = ft / 1000.0;

  screen.setCursor(290, 5);
  screen.print(fps);

  if(forward){
    if(world[int(playerPos.x + playerDir.x * moveSpeed)][int(playerPos.y)] == false) playerPos.x += playerDir.x * moveSpeed;
    if(world[int(playerPos.x)][int(playerPos.y + playerDir.y * moveSpeed)] == false) playerPos.y += playerDir.y * moveSpeed;
  }

  if(backward){
    if(world[int(playerPos.x - playerDir.x * moveSpeed)][int(playerPos.y)] == false) playerPos.x -= playerDir.x * moveSpeed;
    if(world[int(playerPos.x)][int(playerPos.y - playerDir.y * moveSpeed)] == false) playerPos.y -= playerDir.y * moveSpeed;
  }

  if(right){
    double oldDirX = playerDir.x;
    playerDir.x = playerDir.x * cos(-rotSpeed) - playerDir.y * sin(-rotSpeed);
    playerDir.y = oldDirX * sin(-rotSpeed) + playerDir.y * cos(-rotSpeed);

    double oldPlaneX = cameraPlane.x;
    cameraPlane.x = cameraPlane.x * cos(-rotSpeed) - cameraPlane.y * sin(-rotSpeed);
    cameraPlane.y = oldPlaneX * sin(-rotSpeed) + cameraPlane.y * cos(-rotSpeed);
  }

  if(left){
    double oldDirX = playerDir.x;
    playerDir.x = playerDir.x * cos(rotSpeed) - playerDir.y * sin(rotSpeed);
    playerDir.y = oldDirX * sin(rotSpeed) + playerDir.y * cos(rotSpeed);

    double oldPlaneX = cameraPlane.x;
    cameraPlane.x = cameraPlane.x * cos(rotSpeed) - cameraPlane.y * sin(rotSpeed);
    cameraPlane.y = oldPlaneX * sin(rotSpeed) + cameraPlane.y * cos(rotSpeed);
  }
}

