#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(Location, Color) if (GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 12, Color, true);
#define DRAW_SPHERE_SingleFrame (Location, Color) if (GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 12, Color, false, -1.f);
#define DRAW_LINE(StartLocation, EndLocation) if (GetWorld()) DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Black, true, -1.f, 0, 1.f);
#define DRAW_LINE_SingleFrame(StartLocation, EndLocation) if (GetWorld()) DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Black, false, -1.f, 0, 1.f);
#define DRAW_POINT(Location) if (GetWorld()) DrawDebugPoint(GetWorld(), Location, 15.f, FColor::Black, true);
#define DRAW_POINT_SingleFrame(Location) if (GetWorld()) DrawDebugPoint(GetWorld(), Location, 15.f, FColor::Black, false);
#define DRAW_VECTOR(StartLocation, EndLocation) if (GetWorld()) \
   { \
      DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Black, true, -1.f, 0, 1.f); \
      DrawDebugPoint(GetWorld(), EndLocation, 15.f, FColor::Black, true); \
   }
#define DRAW_VECTOR_SingleFrame(StartLocation, EndLocation) if (GetWorld()) \
   { \
      DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Black, false, -1.f, 0, 1.f); \
      DrawDebugPoint(GetWorld(), EndLocation, 15.f, FColor::Black, false); \
   }
#define DRAW_CONE(Location, Rotator) if (GetWorld()) DrawDebugAltCone(GetWorld(), Location, Rotator, 50.f, 50.f, 50.f, FColor::Cyan, true);


