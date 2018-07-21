/*
Definition of weather patterns with pump state, fogger state and led color
*/

struct Weather {
    float dcPump;
    float dcFogger;
    float hueLed;
    float saturationLed;
    float brightnessLed;
} currentWeather;

#define W_ON 1.0
#define W_HALF 0.8
#define W_BRIGHT 1.0
#define W_HBRIGHT 0.7

struct Weather rain  = {W_ON, 0.0, 0.53, 0.3, W_BRIGHT};
struct Weather sunshine  = {0.0, 0.0, 0.07, 0.9, W_BRIGHT};
struct Weather thunderstorm  = {W_ON, 0.0, 0.73, 0.8, W_BRIGHT};
struct Weather fog  = {0.0, W_ON, 0.53, 0.3, W_HBRIGHT};
struct Weather snow  = {W_ON, W_ON, 0.53, 0.3, W_HBRIGHT};
struct Weather night  = {0.0, W_HALF, 0.53, 0.3, 0.0};
struct Weather wind  = {0.0, 0.0, 0.53, 0.3, W_BRIGHT};
struct Weather error = {0.0, 0.0, 1.0, 1.0, W_BRIGHT};
struct Weather noWeather = {0.0, 0.0, 1.0, 1.0, 0.0};
struct Weather allWeather = {W_ON, W_ON, 0.73, 0.8, W_BRIGHT};