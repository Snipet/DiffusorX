#include "visage_ui/frame.h"
#include "visage_ui/scroll_bar.h"

class AboutTextFrame : public visage::Frame {
public:
    AboutTextFrame(const visage::String& str, const visage::Font::Justification& justification = visage::Font::Justification::kLeft);
    ~AboutTextFrame();
    void draw(visage::Canvas& canvas) override;
private:
    visage::String str;
    visage::Font::Justification justification;
};

class AboutPopupFrame : public visage::Frame {
public:
    AboutPopupFrame();
    ~AboutPopupFrame();
    void draw(visage::Canvas& canvas);
private:
    std::unique_ptr<visage::Frame> logo_frame;
    std::unique_ptr<AboutTextFrame> version_frame;
    std::unique_ptr<AboutTextFrame> dsp_frame;
    std::unique_ptr<AboutTextFrame> graphics_frame;
    std::unique_ptr<AboutTextFrame> libraries_frame;
    std::unique_ptr<AboutTextFrame> kissfft_frame;
    std::unique_ptr<AboutTextFrame> visage_frame;
    std::unique_ptr<AboutTextFrame> juce_frame;
    std::unique_ptr<visage::Frame> funk_audio_logo_container;
    std::unique_ptr<visage::Frame> funk_audio_logo;
    visage::Svg funk_audio_logo_svg;
};