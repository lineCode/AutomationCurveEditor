// curve for parameter automation

#ifndef AUTOMATION_CURVE_HPP
#define AUTOMATION_CURVE_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <functional>

namespace AutomationCurve {

struct FloatRange {
    float min = 0;
    float max = 1;
};

using EaseFunctor = std::function<float(const float&)>; // input value is 0..1

struct EaseFunctorFactory {
private:
    static const std::map<std::string, EaseFunctor> _map;

public:
    static EaseFunctor Create(const std::string& name);
    static std::vector<std::string> Names();
    // later:
    // static EaseFunctor CreateWithLUT(const std::string& name, const size_t tableSize);
};

struct SplineControlPoints {
    float startFraction = 0;
    float startValueOffset = 0;
    float endFraction = 1;
    float endValueOffset = 0;
};

// LATER:
//struct SplineFunctor {
//private:
//    SplineControlPoints& _data;
//
//public:
//    SplineFunctor(SplineControlPoints& d)
//        : _data(d)
//    {
//    }
//
//    void operator()(float&, const float&) {}
//};

enum class CycleType {
    Hold,
    Zero,
    Mirror,
    Repeat
};

enum class CurveValueRange {
    Positive, // 0..1 for parameters
    Full // -1..1 for transfer functions
};

enum class LockEdit {
    None,
    LockX,
    LockY,
    LockBoth
};

struct Codec;
struct JSONCodec;

struct ACurve {
protected:
    std::vector<float> _pointPositions;
    std::vector<float> _pointValues;

    std::vector<EaseFunctor> _transitionToPoint; // size, at(0) is ignored
    std::vector<std::string> _typenameOfTransitionToPoint;
    
    std::vector<LockEdit> _pointLock;

    std::map<size_t, SplineControlPoints> _splineControlPoints; // end poind holds both spline control point with relative values

    std::string _defaultTransitionType = "Linear";

    CycleType _cycleType = CycleType::Hold;
    CurveValueRange _valueRange = CurveValueRange::Positive;

    // --- no serialization:
    
    std::vector<size_t> _selectionIdx;
    
    // ---
    
    void _ClampTime(float& t);
    void _ClampValue(float& t);

    friend struct Codec;
    friend struct JSONCodec;
    // ---
public:
    void AddPoint(const float& fract, const float& value);
    void RemovePointAt(const size_t& idx);

    void Clear(); // resets to two points at 0 and 1

    void InitConstant(const float& c); // const value
    void InitTF(); // -1..1

    size_t Size();

    void SetPointLock(const size_t& idx, const LockEdit& l);

    void SetTransitionToPoint(const size_t& idx, const std::string& f);
    void SetAllTransitions(const std::string& f);

    std::string TransitionAt(const size_t& idx);

    const int32_t LeftPointIndexAtFraction(const float& f); // -1 for not found;
    const int32_t RightPointIndexAtFraction(const float& f); // -1 for not found;

    const FloatRange TimeRangeForPoint(const size_t& idx);

    void SetDefaultTransitionType(const std::string& tt) { _defaultTransitionType = tt; }
    const std::string DefaultTransitionType() const { return _defaultTransitionType; }

    std::vector<float> RawValues();
    std::vector<float> RawPositions();
    
    bool IsSelected(const size_t idx);
    size_t SelectionSize() const { return _selectionIdx.size(); }

    // TODO:
    std::vector<std::pair<float,float>> RawPoints();

    const float TimeAt(const size_t& idx);
    const float ValueAt(const size_t& idx);

    const LockEdit LockAt(const size_t& idx);

    float ValueAtFraction(const float& f);

    void SetValue(const size_t& idx, const float& v);
    void SetTime(const size_t& idx, const float& t);
    
    // data
    // TODO:
    const std::vector<float> AsFloatVector(const size_t res);   // resolution
    const ACurve AsLineApproximation(const size_t res);         // constant resolution
    const ACurve AsApproximation(const size_t numsteps);        // num steps
    const std::vector<std::pair<float,float>> AsPoints(const size_t res);

    // edit
    void SelectPoint(const size_t idx);
    void DeselectPoint(const size_t idx);
    void ClearSelection();

    void SelectAll();

    // relative
    void MoveSelectionValue(const float& v);
    void MoveSelectionTime(const float& v);

    void SetSelectionTransitions(const std::string& t);
    void SetSelectionLocks(const LockEdit& l);

    void DeleteSelection();
};

struct Converter{
    const ACurve& _value;
    Converter(const ACurve& v):_value(v){};
    
    const float GetProgressFraction();
    
    const std::vector<float> AsFloatVector(const size_t res);   // resolution
    const ACurve AsLineApproximation(const size_t res);         // constant resolution
    const ACurve AsApproximation(const size_t numsteps);        // num steps
    const std::vector<std::pair<float,float>> AsPoints(const size_t res);
    
    // threaded:
    struct OptionalFloatVector{
        std::vector<float> data;
        bool ready = false;
    };
    
    struct OptionalACurve{
        ACurve data;
        bool ready = false;
    };
    
    struct OptionalPointsVector{
        std::vector<std::pair<float,float>> data;
        bool ready = false;
    };
    
    void Cancel_T();
    
    const void AsFloatVector_T(OptionalFloatVector& obj, const FloatRange& timeRange, const size_t res);   // resolution
    const void AsLineApproximation_T(OptionalACurve& obj, const FloatRange& timeRange,  const size_t res);         // constant resolution
    const void AsApproximation_T(OptionalACurve& obj, const FloatRange& timeRange,  const size_t numsteps);        // num steps
    const void AsPoints_T(OptionalPointsVector& obj, const FloatRange& timeRange, const size_t res);
    
};

struct Editor{
//    std::vector<size_t> _selectionIdx;
    ACurve& _value;
    Editor(ACurve& v):_value(v){};
    
    void SelectPoint(const size_t idx);
    void DeselectPoint(const size_t idx);
    void ClearSelection();

    void SelectAll();

//    size_t SelectionSize() const { return _selectionIdx.size(); }

//    bool IsSelected(const size_t idx);

    // relative
    void MoveSelectionValue(const float& v);
    void MoveSelectionTime(const float& v);

    void SetSelectionTransitions(const std::string& t);
    void SetSelectionLocks(const LockEdit& l);

    void DeleteSelection();
};

struct MultiCurve {
    struct CurveColor {
        unsigned char r = (unsigned char)192;
        unsigned char g = (unsigned char)168;
        unsigned char b = (unsigned char)0;
        
        CurveColor(){}
        CurveColor(const unsigned char r_, const unsigned char g_, const unsigned char b_):r(r_),g(g_),b(b_){}
    };
    
    private:
    std::map<std::string, CurveColor> _colorForCurve;
    std::map<std::string, bool> _isHidden;
    
    public:
    CurveColor defaultColor;
    std::map<std::string, ACurve> curves;
    std::string activeCurve = "";
    
    inline bool HasActiveCurve(){ return curves.find(activeCurve) != curves.end(); }

    void SetHidden(const std::string& name, const bool& v);
    bool IsHidden(const std::string& name); // default false
    
    CurveColor GetColor(const std::string& name);
    void SetColor(const std::string& name,const CurveColor& c );
};

// ---

struct Codec {
    static std::string ToString(const CycleType& ct);
    static std::string ToString(const CurveValueRange& ct);
    static std::string ToString(const LockEdit& ct);

    static bool FromString(CycleType& ct, const std::string& s);
    static bool FromString(CurveValueRange& ct, const std::string& s);
    static bool FromString(LockEdit& ct, const std::string& s);

//    static void ToJSONString(const ACurve& ct, std::string& s);
//    static bool FromJSONString(ACurve& ct, const std::string& s);

    // mc color - json (impl)
    // mc json
};

}; // namespace

#endif
