
#pragma once

#include <metavision/sdk/driver/camera.h>

#include "parameters.hpp"

void readBiasesCam(Metavision::Camera &cam);

bool writeBiasesCam(Metavision::Camera &cam, Bias &params);