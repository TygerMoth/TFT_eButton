#include "TFT_eButton.h"

/***************************************************************************************
** Code for the GFX button UI element
** Grabbed from Adafruit_GFX library and enhanced to handle any label font
****************************************************************************************
*** Modified label datum calculation of _yd to allow button heights as small as 12 px
*** Added Enabled, Visible and Smooth states,
*** with enable(), disable(), show(), hide() and added redraw() methods,
*** and modification to contains() method to process only enabled and visible buttons.
*** Modified Sep 23, 2026 - Larry Coffey
***************************************************************************************/
TFT_eButton::TFT_eButton(TFT_eSPI *tft) {
  _gfx       = tft;
  _xd        = 0;
  _yd        = 0;
  _textdatum = MC_DATUM;
  _label[9]  = '\0';
  _currstate  = false;
  _laststate  = false;
  _enblstate  = false;
  _vsblstate  = false;
  _smthstate  = true;
  _inverted  = false;
}

void TFT_eButton::setPressAction(pressCallback action)
{
  pressAction = action;
}

void TFT_eButton::setReleaseAction(releaseCallback action)
{
  releaseAction = action;
}

// Classic initButton() function: pass center & size
void TFT_eButton::initButton(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize)
{
  // Tweak arguments and pass to the newer initButtonUL() function...
  initButtonUL(x - (w / 2), y - (h / 2), w, h, outline, fill, textcolor, label, textsize);
}

// Newer function instead accepts upper-left corner & size
void TFT_eButton::initButtonUL(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize)
{
  _x1           = x1;
  _y1           = y1;
  _w            = w;
  _h            = h;
  _outlinecolor = outline;
  _outlinewidth = 2;
  _fillcolor    = fill;
  _textcolor    = textcolor;
  _textsize     = textsize;
  strncpy(_label, label, 9);
  _pressTime    = 0xFFFFFFFF;
  _releaseTime  = 0xFFFFFFFF;
}

// Adjust text datum and x, y deltas
void TFT_eButton::setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum) {
  int16_t _yd_min = 5; // added int16_t vartiable _yd_min = 5, for minimum of 12 px height buttons
  _xd        = x_delta;
  _yd        = max(y_delta, _yd_min); // changed from y_delta to max(y_delta, _yd_min)
  _textdatum = datum;
}

void TFT_eButton::drawButton(bool inverted, String long_name) {
  uint16_t fill, outline, text;

  _inverted  = inverted;

  if(!inverted) {
    fill    = _fillcolor;
    outline = _outlinecolor;
    text    = _textcolor;
  } else {
    fill    = _textcolor;
    outline = _outlinecolor;
    text    = _fillcolor;
  }

  uint8_t r = min(_w, _h) / 4; // Corner radius
  _gfx->fillRoundRect(_x1, _y1, _w, _h, r, fill);
  _gfx->drawRoundRect(_x1, _y1, _w, _h, r, outline);

  if (_gfx->textfont == 255) {
    _gfx->setCursor(_x1 + (_w / 8),
                    _y1 + (_h / 4));
    _gfx->setTextColor(text);
    _gfx->setTextSize(_textsize);
    _gfx->print(_label);
  }
  else {
    _gfx->setTextColor(text, fill);
    _gfx->setTextSize(_textsize);

    uint8_t tempdatum = _gfx->getTextDatum();
    _gfx->setTextDatum(_textdatum);
    uint16_t tempPadding = _gfx->getTextPadding();
    _gfx->setTextPadding(0);

    if (long_name == "")
      _gfx->drawString(_label, _x1 + (_w/2) + _xd, _y1 + (_h/2) + _yd); // changed from (_h/2) - 4 + _yd to (_h/2) + _yd
    else
      _gfx->drawString(long_name, _x1 + (_w/2) + _xd, _y1 + (_h/2) + _yd); // after the change above to _yd = min(..., 5)

    _gfx->setTextDatum(tempdatum);
    _gfx->setTextPadding(tempPadding);
  }
  _smthstate = false;
}

void TFT_eButton::drawSmoothButton(bool inverted, int16_t outlinewidth, uint32_t bgcolor, String long_name) {
  uint16_t fill, outline, text;
  if (bgcolor != 0x00FFFFFF) _bgcolor = bgcolor;
  if (outlinewidth >=0) _outlinewidth = outlinewidth;
  _inverted  = inverted;

  if(!inverted) {
    fill    = _fillcolor;
    outline = _outlinecolor;
    text    = _textcolor;
  } else {
    fill    = _textcolor;
    outline = _outlinecolor;
    text    = _fillcolor;
  }

  uint8_t r = min(_w, _h) / 4; // Corner radius
  if (outlinewidth > 0) _gfx->fillSmoothRoundRect(_x1, _y1, _w, _h, r, outline, _bgcolor);
  _gfx->fillSmoothRoundRect(_x1+_outlinewidth, _y1+_outlinewidth, _w-(2*_outlinewidth), _h-(2*_outlinewidth), r-_outlinewidth, fill, outline);

  if (_gfx->textfont == 255) {
    _gfx->setCursor(_x1 + (_w / 8),
                    _y1 + (_h / 4));
    _gfx->setTextColor(text);
    _gfx->setTextSize(_textsize);
    _gfx->print(_label);
  }
  else {
    _gfx->setTextColor(text, fill);
    _gfx->setTextSize(_textsize);

    uint8_t tempdatum = _gfx->getTextDatum();
    _gfx->setTextDatum(_textdatum);
    uint16_t tempPadding = _gfx->getTextPadding();
    _gfx->setTextPadding(0);

    if (long_name == "")
      _gfx->drawString(_label, _x1 + (_w/2) + _xd, _y1 + (_h/2) + _yd); // changed from (_h/2) - 4 + _yd to (_h/2) - 4 + _yd
    else
      _gfx->drawString(long_name, _x1 + (_w/2) + _xd, _y1 + (_h/2) + _yd); // after the change above to _yd = min(..., 5)

    _gfx->setTextDatum(tempdatum);
    _gfx->setTextPadding(tempPadding);
  }
  _smthstate = true;
}

void TFT_eButton::redrawDisabledInternal() {
uint8_t r = min(_w, _h) / 4; // Corner radius
  if (_outlinewidth > 0) _gfx->fillSmoothRoundRect(_x1, _y1, _w, _h, r, TFT_DARKGREY, _bgcolor);
  _gfx->fillSmoothRoundRect(_x1+_outlinewidth, _y1+_outlinewidth, _w-(2*_outlinewidth), _h-(2*_outlinewidth), r-_outlinewidth, TFT_LIGHTGREY, TFT_DARKGREY);

  if (_gfx->textfont == 255) {
    _gfx->setCursor(_x1 + (_w / 8),
                    _y1 + (_h / 4));
    _gfx->setTextColor(TFT_DARKGREY);
    _gfx->setTextSize(_textsize);
    _gfx->print(_label);
  }
  else {
    _gfx->setTextColor(TFT_DARKGREY, TFT_LIGHTGREY);
    _gfx->setTextSize(_textsize);

    uint8_t tempdatum = _gfx->getTextDatum();
    _gfx->setTextDatum(_textdatum);
    uint16_t tempPadding = _gfx->getTextPadding();
    _gfx->setTextPadding(0);

    _gfx->drawString(_label, _x1 + (_w/2) + _xd, _y1 + (_h/2) + _yd); // changed from (_h/2) - 4 + _yd to (_h/2) - 4 + _yd


    _gfx->setTextDatum(tempdatum);
    _gfx->setTextPadding(tempPadding);
  }
}

void TFT_eButton::redraw() {
  if(_smthstate) drawSmoothButton(_inverted, _outlinewidth, _bgcolor, _label);
  else drawButton(_inverted, _label);
}

void TFT_eButton::disable() { // Disable button (prevent interaction)
  _enblstate = false;
  redrawDisabled();
}

void TFT_eButton::enable() { // Enable button (allow interaction)
  _enblstate = true;
  redraw();
}

void TFT_eButton::show() { // Show button (make visible)
  _vsblstate = true;
  redraw();
}

void TFT_eButton::hide() { // Hide button (make invisible)
  _vsblstate = false;
  erase();
}

void TFT_eButton::erase() {  // Erase button
  _gfx->fillRect(_x1, _y1, _w, _h, _bgcolor);
}

void TFT_eButton::toggleEnabled() {  // Toggle enable/disable
  _enblstate = !_enblstate;
  redraw();
}

void TFT_eButton::toggleVisible() {  // Toggle visible/hidden
  _vsblstate = !_vsblstate;
  if (_vsblstate) redraw();
  else erase();
}

void TFT_eButton::redrawDisabled() {  // Draw disabled appearance
    if (_vsblstate) redrawDisabledInternal();
}

bool TFT_eButton::contains(int16_t x, int16_t y) {
  if (!_vsblstate || !_enblstate) return false;
  return ((x >= _x1) && (x < (_x1 + _w)) &&
          (y >= _y1) && (y < (_y1 + _h)));
}

void TFT_eButton::press(bool p) {
  if (_vsblstate && _enblstate) { // ignore press(T/F) if not visible or enabled
    _laststate = _currstate;
    _currstate = p;
  }
}

bool TFT_eButton::isPressed()    { return _currstate; }
bool TFT_eButton::justPressed()  { return (_currstate && !_laststate); }
bool TFT_eButton::justReleased() { return (!_currstate && _laststate); }
