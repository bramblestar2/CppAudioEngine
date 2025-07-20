#pragma once
#include <string>

class ClipEffect {
public:
    virtual void apply(class Clip& clip, float* out, unsigned long frames);
    virtual double modifyStep(double step);
    virtual std::string name() const = 0;
};


class ClipFadeIn : public ClipEffect {
public:
    ClipFadeIn(double durationSeconds,
               std::pair<double,double> cp1 = {0.3, 0.0},
               std::pair<double,double> cp2 = {0.7, 1.0});

    void apply(class Clip& clip, float* out, unsigned long frames) override;
    virtual std::string name() const override;

    void setDuration(double durationSeconds) { m_duration = durationSeconds; }
    void setControlPoints(std::pair<double, double> cp1, std::pair<double, double> cp2) { m_cp1 = cp1; m_cp2 = cp2; }

    double duration() const { return m_duration; }
    std::pair<double, double> controlPointOne() const { return m_cp1; }
    std::pair<double, double> controlPointTwo() const { return m_cp2; }

private:
    double m_duration;
    std::pair<double, double> m_cp1, m_cp2;
};


class ClipFadeOut : public ClipEffect {
public:
    ClipFadeOut(double durationSeconds, 
                std::pair<double,double> cp1 = {0.3, 0.0},
                std::pair<double,double> cp2 = {0.7, 1.0});

    void apply(class Clip& clip, float* out, unsigned long frames) override;
    virtual std::string name() const override;

    void setDuration(double durationSeconds) { m_duration = durationSeconds; }
    void setControlPoints(std::pair<double, double> cp1, std::pair<double, double> cp2) { m_cp1 = cp1; m_cp2 = cp2; }

    double duration() const { return m_duration; }
    std::pair<double, double> controlPointOne() const { return m_cp1; }
    std::pair<double, double> controlPointTwo() const { return m_cp2; }

private:
    double m_duration;
    std::pair<double, double> m_cp1, m_cp2;
};


class ClipLoop : public ClipEffect {
public:
    ClipLoop(bool loop = true);

    void apply(class Clip& clip, float* out, unsigned long frames) override;
    virtual std::string name() const override;

    void setLoop(bool loop) { m_loop = loop; }
    bool loop() const { return m_loop; }

private:
    bool m_loop;
};


class ClipGain : public ClipEffect {
public:
    ClipGain(double gain);

    void apply(class Clip& clip, float* out, unsigned long frames) override;
    virtual std::string name() const override;

    void setGain(double gain) { m_gain = gain; }
    double gain() const { return m_gain; }

private:
    double m_gain;
};
