#include <d3d11.h>
#include <d3dx11tex.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <imgui_internal.h>
#include <functional>
#include <filesystem>
#include <animations.hpp>
#include <colorpicker.h>
#include <string>
#include "unicodes.hpp"

#define col( x, y, z, a ) ImColor{ x, y, z, int( 255 * a * GImGui->Style.Alpha ) }

inline float dpi_scale = 1.f;
inline bool init_fonts = false;

inline ImVec2 SCALE( float x, float y ) {
    return ImVec2{ x, y } * dpi_scale;
}

inline float SCALE( float x ) {
    return x * dpi_scale;
}

#include "fonts.h"

using namespace ImGui;

enum fonts_ {
    font,
    fontb,
    icons,
    size
};

inline std::vector< c_font > fonts( fonts_::size );

#include "notifications.hpp"

inline ID3D11ShaderResourceView* logo;

struct multi_select_item {
    const char* label;
    bool selected = false;

    multi_select_item( const char* _label ) : label{ _label } { };

    operator bool( ) const {
        return selected;
    }
};

struct c_tab {
    const char* label;

    std::vector < const char* > subtabs = { };
    std::vector< std::function< void( ) > > pages = { };
    int cur_subtab = 0;
    int next_page = 0;
};

enum border_ {
    border_left,
    border_right,
    border_top,
    border_bottom,
};

namespace dl {
    namespace window {
        void draw_bg( ImColor col = GetColorU32( ImGuiCol_ChildBg ), float rounding = GImGui->Style.WindowRounding, ImDrawFlags flags = ImDrawFlags_RoundCornersLeft );
        void border( int bord = border_right, ImColor col = GetColorU32( ImGuiCol_Border ) );
    };
};

namespace ui {
    inline ImVec2 size{ 722, 640 };
    inline int next_tab;
    inline bool popup_open = false;
    
    inline float content_anim = 0.f;
    inline float content_anim2 = 0.f;
    inline float content_anim_dest = 1.f;
    inline float content_anim2_dest = 1.f;
    inline int cur_page = 0;
    inline float copied_col[4];
    inline float menu_col[4] { 188 / 255.f, 164 / 255.f, 178 / 255.f, 1.f };

    namespace tabs_manager {
        bool tab( int num );
        void render( float spacing, bool line = false );
    }

    namespace subtabs_manager {
        bool subtab( int num );
        void render( float spacing, bool line = true );
    }

    bool label_btn( const char* label );
    void rotate_start( );
    ImVec2 rotation_center( );
    void rotate_end( float rad, ImVec2 center );
    void handle_alpha_anim( );
    void render_page( );
    void arrow( ImVec2 pos, ImU32 col, ImGuiDir dir, float scale );
    void add_page( int tab, std::function< void( ) > code );
    ImVec2 text_size( fonts_ font, float size, const char* text );
    void styles( );
    void colors( );
    bool color_btn( const char* str_id, float col[4], ImVec2 size, bool color_picker = false );
    bool color_edit( const char* label, float col[4] );
    void add_text( fonts_ font, int size, ImVec2 pos, ImColor col, const char* text, const char* text_end = 0 );

    void child( const char* name, std::function< void( ) > content, ImVec2 size = ImVec2{ 0, 0 } );
    bool binder_ex( const char* label, c_key* key );
    bool binder( const char* label, c_key* key );
    void multi_select( const char* label, std::vector< multi_select_item >& items );
    void settings_btn( const char* str_id, void ( *settings )( ) );

    template < typename T >
    bool slider( const char* label, T* v, T min, T max, const char* format );
    bool slider_int( const char* label, int* v, int min, int max, const char* format = "%d" );
    bool slider_float( const char* label, float* v, float min, float max, const char* format = "%.1f" );
    bool checkbox( const char* label, bool* v, c_key* key = nullptr, std::vector< float* > colors = { }, void( *options )( ) = nullptr, const char* tooltip = 0 );
    bool combo( const char* label, int* v, std::vector< const char* > items );
    void combo_ex( const char* label, const char* buf, std::function< void( ) > popup, bool close_popup = false );
    bool selectable( const char* label, bool selected, const ImVec2& size_arg = ImVec2{ 0, 0 } );
    bool button( const char* label, const ImVec2& size_arg = ImVec2{ 0, 0 } );
    void tooltip( const char* str_id, const char* text, ImVec2 pos );

    inline std::vector< c_tab > tabs { 
        { "aimbot" },
        { "players" },
        { "misc" },
        { "playerlist" },
    };
};

#include "search.hpp"

static inline ImVec2 operator+(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x + rhs, lhs.y + rhs); }
static inline ImVec2 operator-(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x - rhs, lhs.y - rhs); }
static inline ImVec4 operator+(const ImVec4& lhs, const float rhs) { return ImVec4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs); }
static inline ImVec4 operator-(const ImVec4& lhs, const float rhs) { return ImVec4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs); }
static inline ImColor operator/(const ImColor& lhs, const float rhs ) { return ImColor(lhs.Value.x / rhs, lhs.Value.y / rhs, lhs.Value.z / rhs, lhs.Value.w); }