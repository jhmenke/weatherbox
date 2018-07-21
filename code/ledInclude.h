/*
Definition of led strip conversion and color set functions
*/

#define NUM_LED_PIXELS 9
Adafruit_WS2801 ledStrip = Adafruit_WS2801(NUM_LED_PIXELS);

float _hue2rgb(float p, float q, float t)
{
    if (t < 0.0) 
        t += 1.0;
    if(t > 1.0) 
        t -= 1.0;
    if(t < 1.0/6.0) 
        return p + (q - p) * 6.0 * t;
    if(t < 1.0/2.0) 
        return q;
    if(t < 2.0/3.0) 
        return p + (q - p) * (2.0/3.0 - t) * 6.0;

    return p;
}

void setStripColorHSB(float hue, float saturation, float brightness)
{
    float r, g, b;
    float q = brightness < 0.5 ?  brightness * (1.0 + saturation) : brightness + saturation - brightness * saturation;
    float p = 2.0 * brightness - q;
    r = _hue2rgb(p, q, hue + 1.0/3.0);
    g = _hue2rgb(p, q, hue);
    b = _hue2rgb(p, q, hue - 1.0/3.0);

    for (int i=0; i < ledStrip.numPixels(); i++) {
        ledStrip.setPixelColor(i, (byte)(255.0*r), (byte)(255.0*g), (byte)(255.0*b));
        ledStrip.show();
        // delay(50);
    }
}