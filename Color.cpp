#include "Color.hpp"
#include <string>

namespace Spice
{

    ColorManager::ColorManager(float _r, float _g, float _b, float _a) :
        ColorPtr(new Spice::Color(_r, _g, _b, _a)), m_KillColor(true), m_Instances(1) {}

    ColorManager::ColorManager(Color* _col, bool _killColor):
        ColorPtr(_col), m_KillColor(_killColor), m_Instances(1) {}

    void ColorManager::IncInstance()
    {
        m_Instances += 1;
    }

    void ColorManager::DecInstance()
    {
        m_Instances -= 1;

        if (m_Instances == 0)
        {
            if (m_KillColor) delete ColorPtr;
            delete this;
        }
    }



    LuaColor::LuaColor(ColorManager* _manager):
        Manager(_manager), ColorPtr(_manager->ColorPtr)
    {
        Manager->IncInstance();
    }

    LuaColor::LuaColor(Color* _color, bool _killColor):
        Manager(new ColorManager(_color, _killColor)), ColorPtr(Manager->ColorPtr) {}

    LuaColor::LuaColor(float _r, float _g, float _b, float _a) :
        Manager(new ColorManager(_r, _g, _b, _a)), ColorPtr(Manager->ColorPtr) {}

    LuaColor::~LuaColor()
    {
        Manager->DecInstance();
    }

    int LuaColor::lua_FromColor(lua_State* L, Color* _color, bool _killColor)
    {
        LuaColor* color = static_cast<LuaColor*>(lua_newuserdata(L, sizeof(LuaColor)));
        new (color) LuaColor(_color, _killColor);

        luaL_getmetatable(L, "ColorMTT");
        lua_setmetatable(L, -2);

        return 1;
    }

    int LuaColor::lua_FromManager(lua_State* L, ColorManager* _manager)
    {
        LuaColor* color = static_cast<LuaColor*>(lua_newuserdata(L, sizeof(LuaColor)));
        new (color) LuaColor(_manager);

        luaL_getmetatable(L, "ColorMTT");
        lua_setmetatable(L, -2);

        return 1;
    }

    int LuaColor::lua_FromRGB(lua_State* L)
    {
        float r, g, b, a = 1.0f;

        int argAmt = lua_gettop(L);
        if (argAmt == 4)
        {
            r = lua_tonumber(L, 1);
            g = lua_tonumber(L, 2);
            b = lua_tonumber(L, 3);
            a = lua_tonumber(L, 4);
        }
        else if (argAmt == 3)
        {
            r = lua_tonumber(L, 1);
            g = lua_tonumber(L, 2);
            b = lua_tonumber(L, 3);
        }
        else
        {
            LuaThrowSyntaxError(L, "Color.FromRGB(r, g, b, a=1.0) requires 3 or 4 arguments.");
            return 0;
        }

        LuaColor* color = static_cast<LuaColor*>(lua_newuserdata(L, sizeof(LuaColor)));
        new (color) LuaColor(r, g, b, a);

        luaL_getmetatable(L, "ColorMTT");
        lua_setmetatable(L, -2);

        return 1;
    }
    int LuaColor::lua_FromHSV(lua_State* L)
    {
        float h, s, v, a = 1.0f;

        int argAmt = lua_gettop(L);
        if (argAmt == 4)
        {
            h = lua_tonumber(L, 1);
            s = lua_tonumber(L, 2);
            v = lua_tonumber(L, 3);
            a = lua_tonumber(L, 4);
        }
        else if (argAmt == 3)
        {
            h = lua_tonumber(L, 1);
            s = lua_tonumber(L, 2);
            v = lua_tonumber(L, 3);
        }
        else
        {
            LuaThrowSyntaxError(L, "Color.FromHSV(h, s, v, a=1.0) requires 3 or 4 arguments.");
            return 0;
        }

        Color col = Color(h, s, v, a, ColorType::HSV);
        float r = col.r;
        float g = col.g;
        float b = col.b;

        LuaColor* color = static_cast<LuaColor*>(lua_newuserdata(L, sizeof(LuaColor)));
        new (color) LuaColor(r, g, b, a);

        luaL_getmetatable(L, "ColorMTT");
        lua_setmetatable(L, -2);

        return 1;
    }
    int LuaColor::lua_Clone(lua_State* L)
    {
        if (lua_gettop(L) == 1)
        {
            if (lua_isuserdata(L, 1))
            {
                Color* col = static_cast<LuaColor*>(lua_touserdata(L, 1))->ColorPtr;
                Color* new_color = new Color(*col);
                lua_FromColor(L, new_color, true);
                return 1;
            }
        }
        else
        {
            LuaThrowSyntaxError(L, "Color.Clone(color) requires 1 parameter!");
            return 0;
        }
    }

    int LuaColor::lua_Index(lua_State* L)
    {
        Color* col = static_cast<LuaColor*>(lua_touserdata(L, 1))->ColorPtr;
        std::string index = lua_tostring(L, 2);

        if (index == "r")
        {
            lua_pushnumber(L, col->r);
            return 1;
        }
        else if (index == "g")
        {
            lua_pushnumber(L, col->g);
            return 1;
        }
        else if (index == "b")
        {
            lua_pushnumber(L, col->b);
            return 1;
        }
        else if (index == "a")
        {
            lua_pushnumber(L, col->a);
            return 1;
        }
        else
        {
            lua_getglobal(L, "Color");
            lua_pushstring(L, index.c_str());
            lua_rawget(L, -2);

            return 1;
        }
    }
    int LuaColor::lua_NewIndex(lua_State* L)
    {
        Color* col = static_cast<LuaColor*>(lua_touserdata(L, 1))->ColorPtr;
        std::string index = lua_tostring(L, 2);

        if (lua_isnumber(L, 3))
        {
            float value = lua_tonumber(L, 3);
            if (value < 0.0f)
                value = 0.0f;

            if (value > 1.0f)
                value = 1.0f;

            if (index == "r")
                col->r = value;

            else if (index == "g")
                col->g = value;

            else if (index == "b")
                col->b = value;

            else if (index == "a")
                col->a = value;

        }
        else
        {
            LuaThrowSyntaxError(L, "Color properties must be numbers.");
        }

        return 0;

    }
    int LuaColor::lua_ToString(lua_State* L)
    {
        Color* col = static_cast<LuaColor*>(lua_touserdata(L, 1))->ColorPtr;
        std::string str = "Color(" + std::to_string(col->r) + ", " + std::to_string(col->g) + ", " + std::to_string(col->b)
            + ", " + std::to_string(col->a) + ")";

        lua_pushstring(L, str.c_str());
        return 1;
    }
    int LuaColor::lua_Mul(lua_State* L)
    {
        Color* c1 = static_cast<LuaColor*>(lua_touserdata(L, 1))->ColorPtr;

        Color color(1, 1, 1, 1);

        if (lua_isuserdata(L, 2))
        {
            Color* c2 = static_cast<LuaColor*>(lua_touserdata(L, 2))->ColorPtr;

            color.r = c1->r * c2->r;
            color.g = c1->g * c2->g;
            color.b = c1->b * c2->b;
            color.a = c1->a * c2->a;

            lua_pop(L, 2);
        }
        else if (lua_isnumber(L, 2))
        {
            float n = lua_tonumber(L, 2);

            color.r = c1->r * n;
            color.g = c1->g * n;
            color.b = c1->b * n;

            lua_pop(L, 2);
        }

        lua_pushnumber(L, color.r);
        lua_pushnumber(L, color.g);
        lua_pushnumber(L, color.b);
        lua_pushnumber(L, color.a);
        lua_FromRGB(L);
        return 1;
    }
    int LuaColor::lua_GC(lua_State* L)
    {
        LuaColor* lua_color = static_cast<LuaColor*>(lua_touserdata(L, 1));
        lua_color->~LuaColor();

        return 0;
    }

    void LuaColor::lua_RegisterColor(lua_State* L)
    {
        static const luaL_Reg ColorLib[] =
        {
            {"FromRGB", Spice::LuaColor::lua_FromRGB},
            {"FromHSV", Spice::LuaColor::lua_FromHSV},
            {"Clone", Spice::LuaColor::lua_Clone},
            {NULL, NULL}
        };

        luaL_newlib(L, ColorLib);
        lua_setglobal(L, "Color");

        luaL_newmetatable(L, "ColorMTT");

        lua_pushstring(L, "__index");
        lua_pushcfunction(L, Spice::LuaColor::lua_Index);
        lua_settable(L, -3);

        lua_pushstring(L, "__newindex");
        lua_pushcfunction(L, Spice::LuaColor::lua_NewIndex);
        lua_settable(L, -3);

        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, Spice::LuaColor::lua_ToString);
        lua_settable(L, -3);

        lua_pushstring(L, "__mul");
        lua_pushcfunction(L, Spice::LuaColor::lua_Mul);
        lua_settable(L, -3);

        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, Spice::LuaColor::lua_GC);
        lua_settable(L, -3);
    }



    Color::Color()
    {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
        a = 1.0f;
    }
    Color::Color(float _v1, float _v2, float _v3, float _a, ColorType type)
    {
        
        if (_v2 > 1.0f) _v2 = 1.0f;
        if (_v3 > 1.0f) _v3 = 1.0f;
        if (_a > 1.0f) _a = 1.0f;

        if (_v2 < 0.0f) _v2 = 0.0f;
        if (_v3 < 0.0f) _v3 = 0.0f;
        if (_a < 0.0f) _a = 0.0f;

        if (type == ColorType::HSV)
        {
            Color hsv = FromHSV(_v1, _v2, _v3, _a);
            r = hsv.r;
            g = hsv.g;
            b = hsv.b;
            a = hsv.a;
        }
        else
        {
        
            if (_v1 > 1.0f) _v1 = 1.0f;
            if (_v1 < 0.0f) _v1 = 0.0f;

            r = _v1;
            g = _v2;
            b = _v3;
            a = _a;
        }
    }

    bool Color::operator!=(const Color& c)
    {
        if (this->r != c.r || this->g != c.g || this->b != c.b || this->a != c.a)
            return true;

        return false;
    }
    bool Color::operator==(const Color& c)
    {
        if (this->r == c.r && this->g == c.g && this->b == c.b && this->a == c.a)
            return true;

        return false;
    }

    Color Color::Lerp(Color color, Color target, float t)
    {
        float nr = color.r + (target.r - color.r) * t;
        float ng = color.g + (target.g - color.g) * t;
        float nb = color.b + (target.b - color.b) * t;

        return Color(nr, ng, nb, 1.0f);
    }
    Color Color::FromHSV(float _h, float _s, float _v, float _a)
    {
        double      hh, p, q, t, ff;
        long        i;
        float       outR, outG, outB;
        _h *= 360.0f;

        if (_s <= 0.0) {       // < is bogus, just shuts up warnings
            outR = _v;
            outG = _v;
            outB = _v;
            return Color(outR, outG, outB, _a);
        }

        hh = _h;

        while (hh >= 360.0)
            hh -= 360.0;

        hh /= 60.0;
        i = (long)hh;
        ff = hh - i;
        p = _v * (1.0 - _s);
        q = _v * (1.0 - (_s * ff));
        t = _v * (1.0 - (_s * (1.0 - ff)));

        switch (i) {
        case 0:
            outR = _v;
            outG = t;
            outB = p;
            break;
        case 1:
            outR = q;
            outG = _v;
            outB = p;
            break;
        case 2:
            outR = p;
            outG = _v;
            outB = t;
            break;

        case 3:
            outR = p;
            outG = q;
            outB = _v;
            break;
        case 4:
            outR = t;
            outG = p;
            outB = _v;
            break;
        case 5:
        default:
            outR = _v;
            outG = p;
            outB = q;
            break;
        }
        return Color(outR, outG, outB);
    }
    Color Color::FromRGB(float _r, float _g, float _b, float _a)
    {
        return Color(_r, _g, _b, _a);
    }
}
