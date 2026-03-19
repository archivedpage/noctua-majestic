#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"

using namespace ImGui;

bool ui::tabs_manager::tab( int num ) {
    auto label = tabs[num].label;

    auto window = GetCurrentWindow( );
    auto& style = GetStyle( );
    auto id = window->GetID( label );

    ImVec2 size( CalcTextSize( label, 0, 1 ) );
    ImVec2 p = window->DC.CursorPos;
    ImRect bb( p, p + size );

    ItemSize( bb );
    ItemAdd( bb, id );

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    bool selected = num == next_tab;

    if ( pressed && !selected ) {
        content_anim_dest = 0.f;
        next_tab = num;
    }

    struct s {
        float anim = 0;
        float hover = 0;
        float selected = 0;
        ImColor col = 0;
    }; auto& obj = anim_obj( label, 4, s{ } );

    obj.anim = anim( obj.anim, 0.f, 1.f, hovered || selected );
    obj.selected = anim( obj.selected, 0.f, 1.f, selected );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Text ), obj.selected );

    window->DrawList->AddText( bb.Min, obj.col, label, FindRenderedTextEnd( label ) );

    return pressed;
}

void ui::tabs_manager::render( float spacing, bool line ) {
    BeginGroup( );
    {
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { spacing, spacing } );

        for ( int i = 0; i < tabs.size( ); ++i ) {
            tab( i );

            if ( line ) SameLine( );
        }

        PopStyleVar( );
    }
    EndGroup( );
}

bool ui::subtabs_manager::subtab( int num ) {
    auto& label = tabs[cur_page].subtabs[num];

    auto window = GetCurrentWindow( );
    auto& style = GetStyle( );
    auto id = window->GetID( label );

    ImVec2 size( text_size( font, 14, label ) + SCALE( 20, 16 ) );
    ImVec2 p = window->DC.CursorPos;
    ImRect bb( p, p + size );

    ItemSize( bb );
    ItemAdd( bb, id );

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    bool selected = num == tabs[cur_page].next_page;

    if ( pressed && !selected ) {
        content_anim2_dest = 0.f;
        tabs[cur_page].next_page = num;
    }

    struct s {
        float anim = 0;
        float hover = 0;
        float selected = 0;
        ImColor col = 0;
    }; auto& obj = anim_obj( label, 5, s{ } );

    obj.anim = anim( obj.anim, 0.f, 1.f, hovered || selected );
    obj.selected = anim( obj.selected, 0.f, 1.f, selected );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Text ), obj.selected );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, GetColorU32( ImGuiCol_ChildBg, obj.selected ), SCALE( 3 ) );

    add_text( font, 13, bb.Min + SCALE( 10, 8 ), obj.col, label, FindRenderedTextEnd( label ) );

    return pressed;
}

void ui::subtabs_manager::render( float spacing, bool line ) {
    if ( tabs[cur_page].subtabs.empty( ) )
        return;

    SetCursorPos( SCALE( 14, 14 ) );
    BeginGroup( );
    {
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { spacing, spacing } );
        PushStyleVar( ImGuiStyleVar_Alpha, content_anim * GImGui->Style.Alpha );

        for ( int i = 0; i < tabs[cur_page].subtabs.size( ); ++i ) {
            subtab( i );

            if ( line ) SameLine( );
        }

        PopStyleVar( 2 );
    }
    EndGroup( );

    SetCursorPos( { 0, SCALE( 44 ) } );
}

bool ui::label_btn( const char* label ) {
    auto window = GetCurrentWindow( );
    auto& style = GetStyle( );
    auto id = window->GetID( label );

    ImVec2 size( CalcTextSize( label, 0, 1 ) );
    ImVec2 p = window->DC.CursorPos;
    ImRect bb( p, p + size );

    ItemSize( bb );
    ItemAdd( bb, id );

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    struct s {
        float hover = 0;
        float held = 0;
        ImColor col = 0;
    }; auto& obj = anim_obj( label, 5, s{ } );

    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.held = anim( obj.held, 0.f, 1.f, held );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Scheme ), obj.held );

    window->DrawList->AddText( bb.Min, obj.col, label, FindRenderedTextEnd( label ) );

    return pressed;
}

void ui::add_text( fonts_ font, int size, ImVec2 pos, ImColor col, const char* text, const char* text_end ) {
    return GetWindowDrawList( )->AddText( fonts[font].get( size ), fonts[font].get( size )->FontSize, pos, col, text, text_end );
}

int rotation_start_index;
void ui::rotate_start( )
{
    rotation_start_index = GetWindowDrawList( )->VtxBuffer.Size;
}

ImVec2 ui::text_size( fonts_ font, float size, const char* text ) {
    if ( !fonts[font].get( size ) )
        return ImVec2{ 0, 0 };

    return fonts[font].get( size )->CalcTextSizeA( fonts[font].get( size )->FontSize, FLT_MAX, -1, text, FindRenderedTextEnd( text ) );
}

ImVec2 ui::rotation_center( )
{
    ImVec2 l( FLT_MAX, FLT_MAX ), u( -FLT_MAX, -FLT_MAX );

    const auto& buf = GetWindowDrawList( )->VtxBuffer;
    for ( int i = rotation_start_index; i < buf.Size; i++ )
        l = ImMin( l, buf[i].pos ), u = ImMax( u, buf[i].pos );

    return ImVec2( ( l.x + u.x ) / 2, ( l.y + u.y ) / 2 );
}

void ui::rotate_end( float rad, ImVec2 center )
{
    float s = sin( rad ), c = cos( rad );
    center = ImRotate( center, s, c) - center;

    auto& buf = GetWindowDrawList()->VtxBuffer;
    for ( int i = rotation_start_index; i < buf.Size; i++ )
        buf[i].pos = ImRotate( buf[i].pos, s, c ) - center;
}

void ui::handle_alpha_anim( ) {
    content_anim = ImLerp( content_anim, content_anim_dest, 45.f * GetIO( ).DeltaTime );
    content_anim2 = ImLerp( content_anim2, content_anim2_dest, 45.f * GetIO( ).DeltaTime );

    if ( content_anim < 0.001f ) {
        content_anim_dest = 1.f;
        cur_page = next_tab;
    }

    if ( content_anim2 < 0.001f ) {
        content_anim2_dest = 1.f;
        tabs[cur_page].cur_subtab = tabs[cur_page].next_page;
    }
}

void ui::render_page( ) {
    if ( tabs[cur_page].pages.size( ) == 0 || tabs[cur_page].pages.size( ) <= tabs[cur_page].cur_subtab )
        return;

    if ( next_tab != cur_page ) {
        GetCurrentWindow( )->Scroll.y = GetCurrentWindow( )->scroll_y = 0;
        GetCurrentWindow( )->ScrollTarget.y = 0;
    }

    tabs[cur_page].pages[tabs[cur_page].cur_subtab]( );
}

void ui::add_page( int tab, std::function< void( ) > code ) {
    tabs[tab].pages.emplace_back( code );
}

void ui::child( const char* name, std::function< void( ) > content, ImVec2 size ) {
    auto style = GImGui->Style;
    auto w = GetWindowWidth( );

    add_child( cur_page, tabs[cur_page].cur_subtab, name, 0, content );

    PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );
    BeginChild( std::string( name ).append( "main" ).c_str( ), { 0, 0 }, 0, ImGuiWindowFlags_NoScrollbar );
    GetWindowDrawList( )->AddRect( GetWindowPos( ), GetWindowPos( ) + GetWindowSize( ), col( 36, 36, 36, 1.f ), style.ChildRounding );
    SetWindowSize( CalcItemSize( size, w / 2 - style.WindowPadding.x - style.ItemSpacing.x / 2, GetCurrentWindow( )->ContentSize.y ) );
    w = GetWindowWidth( );

    PushStyleVar( ImGuiStyleVar_WindowPadding, SCALE( 10, 14 ) );
    BeginChild( name, { 0, 0 }, 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar );
    SetWindowSize( { w, GetCurrentWindow( )->ContentSize.y + GImGui->Style.WindowPadding.y * 2 } );
    PopStyleVar( );
    PushStyleVar( ImGuiStyleVar_ItemSpacing, SCALE( 10, 10 ) );

    content( );

    PopStyleVar( );
    EndChild( );
    
    auto p = GetWindowPos( );
    add_text( font, 13, p + SCALE( 22, -7 ), GetColorU32( ImGuiCol_Text ), name, FindRenderedTextEnd( name ) );

    EndChild( );
    PopStyleVar( );

    add_text( font, 13, p + SCALE( 22, -7 ), GetColorU32( ImGuiCol_Text ), name, FindRenderedTextEnd( name ) );
}

bool ui::color_btn( const char* str_id, float col[4], ImVec2 size, bool colorpicker ) {
    auto window = GetCurrentWindow( );
    auto& style = GetStyle( );
    auto id = window->GetID( str_id );

    ImVec2 p = window->DC.CursorPos;
    ImRect bb( p, p + size );

    ItemSize( bb );
    ItemAdd( bb, id );

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );
    bool value_changed = false;

    struct s {
        float anim = 0;
        bool open = false;
        float popup_anim = 0.f;
        bool popup = false;
        ImColor col;
    }; auto& obj = anim_obj( str_id, 110, s{ } );

    if ( pressed ) {
        obj.open = !obj.open;
    }

    if ( hovered && IsMouseClicked( 1 ) ) {
        obj.popup = !obj.popup;
    }

    obj.anim = anim( obj.anim, 0.f, 1.f, obj.open );
    obj.popup_anim = anim( obj.popup_anim, 0.f, 1.f, obj.popup );
    obj.col.Value.x = ImLerp( obj.col.Value.x, col[0], GetIO( ).DeltaTime * 17 );
    obj.col.Value.y = ImLerp( obj.col.Value.y, col[1], GetIO( ).DeltaTime * 17 );
    obj.col.Value.z = ImLerp( obj.col.Value.z, col[2], GetIO( ).DeltaTime * 17 );
    obj.col.Value.w = ImLerp( obj.col.Value.w, col[3] * style.Alpha, GetIO( ).DeltaTime * 17 );

    window->DrawList->AddRectFilled( bb.Min, bb.GetCenter( ), col( 144, 144, 144, 1.f ), SCALE( 3 ), ImDrawFlags_RoundCornersTopLeft );
    window->DrawList->AddRectFilled( { bb.GetCenter( ).x, bb.Min.y }, { bb.Max.x, bb.GetCenter( ).y }, col( 211, 211, 211, 1.f ), SCALE( 3 ), ImDrawFlags_RoundCornersTopRight );
    window->DrawList->AddRectFilled( bb.GetCenter( ), bb.Max, col( 144, 144, 144, 1.f ), SCALE( 3 ), ImDrawFlags_RoundCornersBottomRight );
    window->DrawList->AddRectFilled( { bb.Min.x, bb.GetCenter( ).y }, { bb.GetCenter( ).x, bb.Max.y }, col( 211, 211, 211, 1.f ), SCALE( 3 ), ImDrawFlags_RoundCornersBottomLeft );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, obj.col, SCALE( 3 ) );

    if ( obj.anim > 0.01f ) {
        PushStyleColor( ImGuiCol_WindowBg, GetColorU32( ImGuiCol_FrameBg ) );
        PushStyleVar( ImGuiStyleVar_Alpha, obj.anim * GImGui->Style.Alpha );
        PushStyleVar( ImGuiStyleVar_WindowRounding, style.FrameRounding );
        PushStyleVar( ImGuiStyleVar_ItemSpacing, SCALE( 6, 6 ) );
        PushStyleVar( ImGuiStyleVar_WindowPadding, SCALE( 6, 6 ) );
        PushStyleVar( ImGuiStyleVar_WindowBorderSize, 1 );
        Begin( str_id, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove );
        {
            SetWindowSize( SCALE( 154, 144 ) );
            SetWindowPos( { bb.Max.x - SCALE( 154 ), bb.Max.y + SCALE( 10 ) + SCALE( 20 ) * ( 1.f - obj.anim ) } );

            BringWindowToFocusFront( GetCurrentWindow( ) );
            BringWindowToDisplayFront( GetCurrentWindow( ) );

            if ( !IsWindowHovered( ) && IsMouseClicked( 0 ) && !hovered ) obj.open = false;

            value_changed = color_picker( str_id, col );
        }
        End( );
        PopStyleVar( 5 );
        PopStyleColor( );
    }

    if ( obj.popup_anim > 0.01f ) {
        PushStyleVar( ImGuiStyleVar_WindowRounding, SCALE( 3 ) );
        PushStyleVar( ImGuiStyleVar_WindowBorderSize, 1 );
        PushStyleVar( ImGuiStyleVar_WindowPadding, SCALE( 10, 10 ) );
        PushStyleVar( ImGuiStyleVar_ItemSpacing, SCALE( 10, 6 ) );
        PushStyleVar( ImGuiStyleVar_Alpha, obj.popup_anim * GImGui->Style.Alpha );
        PushStyleColor( ImGuiCol_WindowBg, GetColorU32( ImGuiCol_FrameBg ) );
        Begin( str_id, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiColumnsFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize );
        {
            SetWindowPos( { bb.Max.x - GetWindowWidth( ), bb.Max.y + SCALE( 5 ) + SCALE( 10 ) * ( 1.f - obj.popup_anim ) } );

            BringWindowToFocusFront( GetCurrentWindow( ) );
            BringWindowToDisplayFront( GetCurrentWindow( ) );

            if ( !IsWindowHovered( ) && IsMouseClicked( 0 ) && !hovered ) obj.popup = false;

            if ( label_btn( std::string( "copy##" ).append( str_id ).c_str( ) ) ) {
                copied_col[0] = col[0];
                copied_col[1] = col[1];
                copied_col[2] = col[2];
                copied_col[3] = col[3];
                obj.popup = false;
                notify::add( "color was copied", notify::notify_info );
            }

            if ( label_btn( std::string( "paste##" ).append( str_id ).c_str( ) ) ) {
                col[0] = copied_col[0];
                col[1] = copied_col[1];
                col[2] = copied_col[2];
                col[3] = copied_col[3];
                obj.popup = false;
                notify::add( "color was pasted", notify::notify_info );
            }
        }
        End( );
        PopStyleColor( );
        PopStyleVar( 5 );
    }

    return value_changed;
}

bool ui::color_edit( const char* label, float col[4] ) {
    if ( !add_widget( label ) ) return false;

    GetWindowDrawList( )->AddText( GetCurrentWindow( )->DC.CursorPos + ImVec2{ 0, SCALE( 6.5f ) - GImGui->FontSize / 2 }, GetColorU32( ImGuiCol_Text ), label, FindRenderedTextEnd( label ) );
    Dummy( { 0, 0 } );
    SameLine( CalcItemWidth( ) - SCALE( 3 ) );
    return color_btn( label, col, SCALE( 13, 13 ), true );
}

bool selectable_text( const char* label, bool selected ) {
    auto window = GetCurrentWindow( );
	auto id = window->GetID( label );
	auto& style = GetStyle( );

    struct s {
        float anim;
        float hover;
        ImColor col;
    }; auto& obj = anim_obj( label, 023, s{ } );

    ImVec2 pos = window->DC.CursorPos;
	ImVec2 label_size = CalcTextSize( label, 0, 1 );
	ImRect bb( window->DC.CursorPos, window->DC.CursorPos + CalcTextSize( label, 0, 1 ) );

    ItemSize( bb );
	ItemAdd( bb, id );

	bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    obj.anim = anim( obj.anim, 0.f, 1.f, selected );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Scheme ), obj.anim );

    window->DrawList->AddText( bb.Min, obj.col, label, FindRenderedTextEnd( label ) );

    return pressed;
}

bool ui::binder_ex( const char* label, c_key* bind ) {

    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;

    if ( !add_widget( label ) ) return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( label );
    ImGuiIO& io = g.IO;

    std::string buf_display = "key: none";
    if ( bind->key != 0 && g.ActiveId != id )
        buf_display = std::string( "key: " ).append( KEY_NAMES[bind->key] );
    else if ( g.ActiveId == id )
        buf_display = "key: ...";

    const ImVec2 key_size = CalcTextSize( buf_display.c_str( ) );

    struct s {
        float anim = 0;
        float hover = 0;
        float active = 0;
        ImColor col = 0;
        float bind_w = 0;
        bool open = false;
        float open_anim = 0.f;
    }; auto& obj = anim_obj( label, 6, s{ } );

    obj.bind_w = ImLerp( obj.bind_w, key_size.x, GetIO( ).DeltaTime * 18.f );

    const ImRect total_bb( window->DC.CursorPos, window->DC.CursorPos + ImVec2{ CalcTextSize( label, 0, 1 ).x > 0 ? CalcItemWidth( ) : ( int )obj.bind_w + g.Style.FramePadding.x * 2, key_size.y + g.Style.FramePadding.y * 2 } );
    const ImRect bb( total_bb.Max - ImVec2{ ( int )obj.bind_w + g.Style.FramePadding.x * 2, key_size.y + g.Style.FramePadding.y * 2 }, total_bb.Max );

    ItemSize( total_bb, style.FramePadding.y);
    ItemAdd( total_bb, id, &bb );

    const bool hovered = ItemHoverable( bb, id, 0 );

    const bool SHOULD_EDIT = hovered && io.MouseClicked[0];

    if ( SHOULD_EDIT ) {
        if ( g.ActiveId != id ) {
            memset( io.MouseDown, 0, sizeof( io.MouseDown ) );
            memset( io.KeysDown, 0, sizeof( io.KeysDown ) );
            bind->key = 58;
        }

        SetActiveID( id, window );
        FocusWindow( window );
    }
    else if ( !hovered && io.MouseClicked[0] && g.ActiveId == id ) {
        ClearActiveID( );
        bind->key = 0;
    }

    bool value_changed = false;
    int key = bind->key;

    if ( g.ActiveId == id ) {
        for ( auto i = 0; i < 5; i++ ) {
            if (io.MouseDown[ i ]) {
                switch ( i ) {
                case 0:
                    key = VK_LBUTTON;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                }
                value_changed = true;
                ClearActiveID( );
            }
        }

        if ( !value_changed ) {
            for ( auto i = VK_BACK; i <= VK_RMENU; i++ ) {
                if ( io.KeysDown[i] ) {
                    key = i;
                    value_changed = true;
                    ClearActiveID( );
                }
            }
        }

        if (IsKeyPressedMap( ImGuiKey_Escape )) {
            bind->key = 0;
            ClearActiveID( );
        }
        else
            bind->key = key;
    }

    if ( hovered && IsMouseClicked( 1 ) ) obj.open = !obj.open;

    obj.open_anim = anim( obj.open_anim, 0.f, 1.f, obj.open );
    obj.anim = anim( obj.anim, 0.f, 1.f, g.ActiveId == id || hovered );
    obj.active = anim( obj.active, 0.f, 1.f, g.ActiveId == id );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Scheme ), obj.active );

    PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1 );
    RenderFrame( bb.Min, bb.Max, GetColorU32( ImGuiCol_FrameBg ), 1, style.FrameRounding );
    PopStyleVar( );

    window->DrawList->AddText( total_bb.Min + ImVec2{ 0, total_bb.GetHeight( ) / 2 - GImGui->FontSize / 2 }, GetColorU32( ImGuiCol_Text ), label, FindRenderedTextEnd( label ) );

    window->DrawList->AddText( bb.Min + style.FramePadding, obj.col, buf_display.c_str( ) );

    if ( obj.open_anim > 0.01f ) {
        PushStyleVar( ImGuiStyleVar_WindowRounding, SCALE( 3 ) );
        PushStyleVar( ImGuiStyleVar_WindowBorderSize, 1 );
        PushStyleVar( ImGuiStyleVar_WindowPadding, SCALE( 10, 10 ) );
        PushStyleVar( ImGuiStyleVar_ItemSpacing, SCALE( 10, 6 ) );
        PushStyleVar( ImGuiStyleVar_Alpha, obj.open_anim * GImGui->Style.Alpha );
        PushStyleColor( ImGuiCol_WindowBg, GetColorU32( ImGuiCol_FrameBg ) );
        Begin( label, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiColumnsFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize );
        {
            SetWindowPos( { bb.Max.x - GetWindowWidth( ), bb.Max.y + SCALE( 5 ) + SCALE( 10 ) * ( 1.f - obj.open_anim ) } );

            BringWindowToFocusFront( GetCurrentWindow( ) );
            BringWindowToDisplayFront( GetCurrentWindow( ) );

            if ( !IsWindowHovered( ) && ( IsMouseClicked( 0 ) || IsMouseClicked( 1 ) ) && !hovered ) obj.open = false;

            if ( selectable_text( std::string( "hold" ).append( CalcTextSize( label, 0, 1 ).x > 0 ? "##" : "" ).append( label ).c_str( ), bind->mode == 0 ) ) { bind->mode = 0; obj.open = false; }
            if ( selectable_text( std::string( "toggle" ).append( CalcTextSize( label, 0, 1 ).x > 0 ? "##" : "" ).append( label ).c_str( ), bind->mode == 1 ) ) { bind->mode = 1; obj.open = false; }
        }
        End( );
        PopStyleColor( );
        PopStyleVar( 5 );
    }

    return value_changed;
}

bool ui::binder( const char* label, c_key* bind ) {
    PushFont( fonts[font].get( 13 ) );
    PushStyleVar( ImGuiStyleVar_FramePadding, SCALE( 5, 2 ) );
    PushStyleVar( ImGuiStyleVar_FrameRounding, SCALE( 2 ) );
    bool result = binder_ex( label, bind );
    PopStyleVar( 2 );
    PopFont( );
    return result;
}

void ui::multi_select( const char* label, std::vector< multi_select_item >& items ) {
	auto& style = GetStyle( );

    std::string buf;

    buf.clear( );
    for ( size_t i = 0; i < items.size( ); ++i ) {
        if ( items[i] ) {
            buf += items[i].label;
            buf += ", ";
        }
    }

    if ( !buf.empty( ) ) {
        buf.resize( buf.size( ) - 2 );
    }

    if ( CalcTextSize( buf.c_str( ) ).x > SCALE( 100 ) ) {
        for ( int i = 0; i < buf.size( ) - 1; ++i ) {
            if ( CalcTextSize( buf.substr( 0, i + 1 ).c_str( ) ).x > SCALE( 100 ) ) {
                buf.resize( i );
                if ( buf[buf.size( ) - 1] == ',' ) {
                    buf.resize( buf.size( ) - 1 );
                }
                buf.append( ".." );
            }
        }
    }

    combo_ex( label, buf.c_str( ), [&]( ) {
        for ( int i = 0; i < items.size( ); ++i ) {
            if ( selectable( items[i].label, items[i], { GetWindowWidth( ), GImGui->FontSize } ) ) {
                items[i].selected = !items[i];
            }
        }    
    } );
}

void ui::arrow( ImVec2 pos, ImU32 col, ImGuiDir dir, float scale ) {
    float thickness = ImMax(scale / 5.0f, 1.0f);
    scale -= thickness * 0.5f;
    pos += ImVec2(thickness * 0.25f, thickness * 0.25f);

    float third = scale;

    float rad = ( dir == ImGuiDir_Right ? IM_PI / 4 : dir == ImGuiDir_Down ? IM_PI / -4 : dir == ImGuiDir_Left ? IM_PI * 1.25f : IM_PI * -1.25f );

    rotate_start( );

    GetWindowDrawList( )->PathLineTo(ImVec2(pos.x - third, pos.y));
    GetWindowDrawList( )->PathLineTo(pos);
    GetWindowDrawList( )->PathLineTo(ImVec2(pos.x, pos.y + third));

    GetWindowDrawList( )->PathStroke(col, 0, thickness);

    rotate_end( rad, rotation_center( ) );
}

void ui::settings_btn( const char* str_id, void ( *settings )( ) ) {
    auto window = GetCurrentWindow( );
    auto id = window->GetID( str_id );

    ImRect bb{ window->DC.CursorPos, window->DC.CursorPos + SCALE( 14, 14 ) };

    ItemSize( bb );
    ItemAdd( bb, id );

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    struct s {
        float hover = 0;
        float anim = 0;
        ImColor col = 0;
        bool open = false;
    }; auto& obj = anim_obj( str_id, 521, s{ } );

    if ( pressed ) obj.open = !obj.open;

    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.anim = anim( obj.anim, 0.f, 1.f, obj.open );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Text ), obj.anim );

    add_text( icons, 12, bb.Min, obj.col, settings_3_fill );

    if ( obj.anim > 0.01f ) {
        PushStyleVar( ImGuiStyleVar_Alpha, obj.anim * GImGui->Style.Alpha );
        PushStyleVar( ImGuiStyleVar_WindowRounding, GImGui->Style.FrameRounding );
        PushStyleVar( ImGuiStyleVar_ItemSpacing, SCALE( 14, 14 ) );
        PushStyleVar( ImGuiStyleVar_WindowPadding, SCALE( 14, 14 ) );
        PushStyleVar( ImGuiStyleVar_WindowBorderSize, 1 );
        PushStyleColor( ImGuiCol_WindowBg, GetColorU32( ImGuiCol_FrameBg ) );
        PushFont( fonts[font].get( 13 ) );
        Begin( str_id, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize );
        {
            SetWindowPos( { bb.Max.x - GetWindowWidth( ), bb.Max.y + SCALE( 10 ) + SCALE( 20 ) * ( 1.f - obj.anim ) } );

            BringWindowToFocusFront( GetCurrentWindow( ) );
            BringWindowToDisplayFront( GetCurrentWindow( ) );

            if ( !IsWindowHovered( ) && IsMouseClicked( 0 ) && !hovered ) obj.open = false;

            GetWindowDrawList( )->AddText( GetCurrentWindow( )->DC.CursorPos, GetColorU32( ImGuiCol_Text ), str_id, FindRenderedTextEnd( str_id ) );
            GetWindowDrawList( )->AddRectFilled( GetWindowPos( ) + ImVec2{ 0, GImGui->Style.WindowPadding.y * 2 + GImGui->FontSize }, GetWindowPos( ) + ImVec2{ GetWindowWidth( ), GImGui->Style.WindowPadding.y * 2 + GImGui->FontSize + 1 }, GetColorU32( ImGuiCol_Border ) );
            Dummy( CalcTextSize( str_id, 0, 1 ) );

            PushItemFlag( ImGuiItemFlags_Search, true );
            PushItemWidth( SCALE( 230 ) );
            SetCursorPosY( GImGui->FontSize + GImGui->Style.WindowPadding.y * 3 );

            settings( );

            PopItemWidth( );
            PopItemFlag( );
        }
        End( );
        PopFont( );
        PopStyleColor( );
        PopStyleVar( 5 );
    }
}

bool ui::button( const char* label, const ImVec2& size_arg ) {
    auto window = GetCurrentWindow( );
	auto id = window->GetID( label );
	auto style = GetStyle( );

    auto label_size = CalcTextSize( label, 0, 1 );

    ImRect bb( window->DC.CursorPos, window->DC.CursorPos + CalcItemSize( size_arg, CalcItemWidth( ) * 185 / 300.f, GetFrameHeight( ) ) );

	ItemSize( bb );
	ItemAdd( bb, id );

	bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    struct s {
        float hover = 0;
        float held = 0;
        float anim = 0;
        ImColor col = 0;
        ImColor label_col = 0;
    }; auto& obj = anim_obj( label, 2, s{ } );

    obj.anim = anim( obj.anim, 0.f, 1.f, hovered || held );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.held = anim( obj.held, 0.f, 1.f, held );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_Button ), GetColorU32( ImGuiCol_ButtonHovered ), obj.hover ), GetColorU32( ImGuiCol_ButtonActive ), obj.held );
    obj.label_col = col_anim( GetColorU32( ImGuiCol_Text ), GetColorU32( ImGuiCol_FrameBg ), obj.anim );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, obj.col, style.FrameRounding );
    window->DrawList->AddRect( bb.Min, bb.Max, GetColorU32( ImGuiCol_Border, 1.f - obj.anim ), style.FrameRounding );

    window->DrawList->AddText( bb.GetCenter( ) - label_size / 2, obj.label_col, label );

    return pressed;
}

bool ui::selectable( const char* label, bool selected, const ImVec2& size_arg ) {
    auto window = GetCurrentWindow( );
	auto id = window->GetID( label );
	auto& style = GetStyle( );

    struct s {
        float anim;
        float hover;
        ImColor col;
    }; auto& obj = anim_obj( label, 0, s{ } );

    ImVec2 pos = window->DC.CursorPos;
	ImVec2 label_size = CalcTextSize( label, 0, 1 );
	ImRect bb( window->DC.CursorPos, window->DC.CursorPos + CalcItemSize( size_arg, GetWindowWidth( ), GetFrameHeight( ) ) );

    ItemSize( bb );
	ItemAdd( bb, id );

	bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    obj.anim = anim( obj.anim, 0.f, 1.f, selected );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Scheme ), obj.anim );

    //window->DrawList->AddRectFilled( bb.Min, bb.Max, GetColorU32( ImGuiCol_FrameBgHovered, obj.anim ) );

    window->DrawList->AddText( { bb.Min.x + SCALE( 12 ), bb.GetCenter( ).y - GImGui->FontSize / 2 }, obj.col, label, FindRenderedTextEnd( label ) );

    return pressed;
}

void ui::combo_ex( const char* label, const char* buf, std::function< void( ) > popup, bool close_popup ) {
    auto window = GetCurrentWindow( );
	auto id = window->GetID( label );
	auto& style = GetStyle( );

    if ( !add_widget( label ) ) return;

    struct s {
        float anim;
        float rad;
        float hover;
        float open;
        ImColor col;
    }; auto& obj = anim_obj( label, 0, s{ } );

    ImVec2 pos = window->DC.CursorPos;
	ImVec2 label_size = CalcTextSize( label, 0, 1 );
	ImRect total_bb( pos, pos + ImVec2( CalcItemWidth( ) * 185 / 300.f, SCALE( 14 ) + style.ItemInnerSpacing.y + SCALE( 24 ) ) );
	ImRect bb( total_bb.Max - ImVec2{ total_bb.GetWidth( ), SCALE( 24 ) }, total_bb.Max );

    ItemSize( total_bb );
	ItemAdd( total_bb, id );

	bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    if ( pressed ) obj.open = !obj.open;

    obj.anim = anim( obj.anim, 0.f, 1.f, obj.open );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.rad = anim( obj.rad, IM_PI / 2, IM_PI * 1.5f, obj.open );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Text ), obj.anim );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, GetColorU32( ImGuiCol_FrameBg ), SCALE( 2 ), !obj.open ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersTop );
    window->DrawList->AddRect( bb.Min, bb.Max, GetColorU32( ImGuiCol_Border ), SCALE( 2 ), !obj.open ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersTop );

    rotate_start( );
    arrow( { bb.Max.x - SCALE( 13 ), bb.GetCenter( ).y - SCALE( 2 ) - obj.anim }, col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Scheme ), obj.anim ), ImGuiDir_Down, 5 );
    rotate_end( obj.rad, rotation_center( ) );

    add_text( font, 13, total_bb.Min, GetColorU32( ImGuiCol_Text ), label, FindRenderedTextEnd( label ) );
    add_text( font, 13, { bb.Min.x + SCALE( 8 ), bb.GetCenter( ).y - SCALE( 6.f ) }, GetColorU32( ImGuiCol_Text ), buf, FindRenderedTextEnd( buf ) );
    //window->DrawList->AddText( { bb.Min.x + 12, bb.GetCenter( ).y - GImGui->FontSize / 2 }, obj.col, buf, FindRenderedTextEnd( buf ) );

    if ( obj.anim > 0.05f ) {
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 7 } );
        PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );
        PushStyleVar( ImGuiStyleVar_Alpha, obj.anim * GImGui->Style.Alpha );
        Begin( label, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground );
        {
            SetWindowSize( { bb.GetWidth( ), ( GetCurrentWindow( )->ContentSize.y + style.ItemSpacing.y ) * obj.anim } );
            SetWindowPos( { bb.Min.x, bb.Max.y } );

            BringWindowToFocusFront( GetCurrentWindow( ) );
            BringWindowToDisplayFront( GetCurrentWindow( ) );

            GetWindowDrawList( )->AddRectFilled( GetWindowPos( ), GetWindowPos( ) + GetWindowSize( ), GetColorU32( ImGuiCol_FrameBg ), style.FrameRounding, ImDrawFlags_RoundCornersBottom );
            GetWindowDrawList( )->AddRect( bb.Min, GetWindowPos( ) + GetWindowSize( ), GetColorU32( ImGuiCol_Border ), style.FrameRounding, ImDrawFlags_RoundCornersBottom );

            if ( !close_popup ) {
                if ( !IsWindowHovered( ) && IsMouseClicked( 0 ) && !hovered ) obj.open = false;
            }

            SetCursorPosY( style.ItemSpacing.y );
            BeginGroup( );
            {
                popup( );
            }
            EndGroup( );

            if ( close_popup ) {
                if ( IsMouseReleased( 0 ) && obj.anim > 0.9f ) obj.open = false;
            }
        }
        End( );
        PopStyleVar( 3 );
    }
}

template < typename T >
bool ui::slider( const char* label, T* v, T min, T max, const char* format ) {
    auto window = GetCurrentWindow( );
    auto id = window->GetID( label );
    auto& style = GImGui->Style;

    ImRect total_bb{ window->DC.CursorPos, window->DC.CursorPos + ImVec2{ CalcItemWidth( ) * 185 / 300.f, SCALE( 14 ) + style.ItemInnerSpacing.y + SCALE( 6 ) } };
    ImRect bb{ { total_bb.Min.x, total_bb.Max.y - SCALE( 6 ) }, total_bb.Max };

    ItemSize( total_bb );
    ItemAdd( total_bb, id );

    bool result = false;
    
    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    struct s {
        float anim;
        float hover;
        float held;
        float val_anim;
        ImColor col;
    }; auto& obj = anim_obj( label, 032, s{ } );

    obj.anim = anim( obj.anim, 0.f, 1.f, hovered || held );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.held = anim( obj.held, 0.f, 1.f, held );
    obj.col = col_anim( col_anim( col( 101, 105, 117, 1.f ), col( 101, 105, 117, 0.6f ), obj.hover ), GetColorU32( ImGuiCol_Text ), obj.held );
    obj.val_anim = ImLerp( obj.val_anim, ( ImClamp( *v, min, max ) - min * 1.f ) / ( max - min ) * bb.GetWidth( ), GetIO( ).DeltaTime * 17 );

    if ( held ) {
        *v = ImClamp( T( min + ( GetIO( ).MousePos.x - bb.Min.x ) / bb.GetWidth( ) * ( max - min ) ), min, max );
        result = true;
    }

    char buf[64];
    ImFormatString( buf, sizeof( buf ), format, *v );

    window->DrawList->AddRectFilled( bb.Min + SCALE( 0, 1 ), bb.Max - SCALE( 0, 1 ), GetColorU32( ImGuiCol_FrameBg ), SCALE( 2 ) );
    window->DrawList->AddRect( bb.Min + SCALE( 0, 1 ), bb.Max - SCALE( 0, 1 ), GetColorU32( ImGuiCol_Border ), SCALE( 2 ) );
    window->DrawList->AddRectFilled( bb.Min, { bb.Min.x + obj.val_anim, bb.Max.y }, GetColorU32( ImGuiCol_Scheme, 1.f - 0.4f * obj.held ), SCALE( 2 ) );

    add_text( font, 13, total_bb.Min, GetColorU32( ImGuiCol_Text ), label, FindRenderedTextEnd( label ) );
    add_text( font, 13, { total_bb.Min.x + text_size( font, 13, label ).x + SCALE( 8 ), total_bb.Min.y }, obj.col, buf );

    return result;
}

bool ui::slider_int( const char* label, int* v, int min, int max, const char* format ) {
    return slider< int >( label, v, min, max, format );
}

bool ui::slider_float( const char* label, float* v, float min, float max, const char* format ) {
    return slider< float >( label, v, min, max, format );
}

bool ui::checkbox( const char* label, bool* v, c_key* key, std::vector< float* > col, void( *options )( ), const char* tooltip ) {
    auto window = GetCurrentWindow( );
	auto id = window->GetID( label );
	auto style = GetStyle( );
    float w = CalcItemWidth( );

    if ( !add_widget( label ) ) return false;

	float square_sz = SCALE( 13 );
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 label_size = CalcTextSize( label, 0, 1 );
	ImRect bb( pos, pos + ImVec2( square_sz + style.ItemInnerSpacing.x + label_size.x, label_size.y ) );
	ImRect click_bb( { bb.Min.x, bb.GetCenter( ).y - square_sz / 2 }, { bb.Min.x + square_sz, bb.GetCenter( ).y + square_sz / 2 } );

	ItemSize( bb );
	ItemAdd( bb, id );

	bool hovered, held;
	bool pressed = ButtonBehavior( bb, id, &hovered, &held );
    bool value_changed = false;

    if ( pressed ) {
        *v = !*v;
        value_changed = true;
    }

    struct s {
        float anim = 0;
        float hover = 0;
        float enabled = 0;
        ImColor col = 0;
        float bind_w = 0;
    }; auto& obj = anim_obj( label, 0, s{ } );

    obj.anim = anim( obj.anim, 0.f, 1.f, hovered || *v );
    obj.hover = anim( obj.hover, 0.f, 1.f, hovered );
    obj.enabled = anim( obj.enabled, 0.f, 1.f, *v );
    obj.col = col_anim( col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.hover ), GetColorU32( ImGuiCol_Text ), obj.enabled );

	window->DrawList->AddRectFilled( click_bb.Min, click_bb.Max, GetColorU32( ImGuiCol_FrameBg ), SCALE( 3 ) );
	window->DrawList->AddRect( click_bb.Min, click_bb.Max, GetColorU32( ImGuiCol_Border ), SCALE( 3 ) );
	window->DrawList->AddRectFilled( click_bb.Min, click_bb.Max, GetColorU32( ImGuiCol_Scheme, obj.enabled ), SCALE( 3 ) );

	window->DrawList->AddText( { bb.Min.x + square_sz + style.ItemInnerSpacing.x, bb.GetCenter( ).y - label_size.y / 2 }, obj.col, label, FindRenderedTextEnd( label ) );

    if ( key ) {
        pos = window->DC.CursorPos;

        obj.bind_w = ImLerp( obj.bind_w, ui::text_size( font, 13, std::string( "key: " ).append( KEY_NAMES[key->key] ).c_str( ) ).x, GetIO( ).DeltaTime * 18.f );

        window->DC.CursorPos = ImVec2{ bb.Min.x + w - ( int )obj.bind_w - GImGui->Style.FramePadding.x * 2 - SCALE( 26 ) * col.size( ), bb.GetCenter( ).y - SCALE( 17 ) / 2 };
        BeginChild( std::string( label ).append( "binder" ).c_str( ), { GImGui->Style.FramePadding.x * 2 + ( int )obj.bind_w, SCALE( 17 ) }, 0, ImGuiWindowFlags_NoBackground );
        ui::binder( std::string( "##" ).append( label ).append( "binder" ).c_str( ), key );
        EndChild( );

        window->DC.CursorPos = pos;
    }

    if ( col.size( ) > 0 ) {
        pos = window->DC.CursorPos;

        window->DC.CursorPos = ImVec2{ bb.Min.x + w - SCALE( 21 ) * col.size( ) + SCALE( 8 ), bb.GetCenter( ).y - SCALE( 6.5f ) };

        BeginChild( std::string( label ).append( "colors" ).c_str( ), { SCALE( 21 ) * col.size( ) - SCALE( 8 ), SCALE( 13 ) }, 0, ImGuiWindowFlags_NoBackground );

        for ( int i = 0; i < col.size( ); ++i ) {
            ui::color_btn( std::string( label ).append( "##" ).append( std::to_string( i ) ).c_str( ), col[i], SCALE( 13, 13 ) );
            SameLine( 0, SCALE( 8 ) );
        }

        EndChild( );

        window->DC.CursorPos = pos;
    }

    if ( options != 0 ) {
        pos = window->DC.CursorPos;

        window->DC.CursorPos = ImVec2{ bb.Min.x + w - SCALE( 14 ) - SCALE( 21 ) * col.size( ) - ( obj.bind_w + SCALE( 18 ) ) * ( bool )key, bb.GetCenter( ).y - SCALE( 7 ) };

        BeginChild( std::string( label ).append( "settings" ).c_str( ), SCALE( 14, 14 ), 0, ImGuiWindowFlags_NoBackground );
        ui::settings_btn( std::string( label ).append( "##settings" ).c_str( ), options );
        EndChild( );

        window->DC.CursorPos = pos;
    }

    if ( tooltip != 0 ) {
        ui::tooltip( std::string( label ).append( "tooltip" ).c_str( ), tooltip, { click_bb.Max.x + CalcTextSize( label, 0, 1 ).x + SCALE( 10 ) + style.ItemInnerSpacing.x, bb.GetCenter( ).y - SCALE( 7 ) } );
    }

    return pressed;
};

void ui::tooltip( const char* str_id, const char* text, ImVec2 pos ) {
    auto window = GetCurrentWindow( );
    auto id = window->GetID( str_id );

    ImRect bb{ pos, pos + SCALE( 13, 13 ) };

    struct s {
        float anim = 0;
        float time = 0;
        ImColor col = 0;
    }; auto& obj = anim_obj( str_id, 521, s{ } );

    if ( IsMouseHoveringRect( bb.Min, bb.Max ) ) {
        obj.time += GetIO( ).DeltaTime;
    } else {
        obj.time = 0;
    }

    obj.anim = anim( obj.anim, 0.f, 1.f, IsMouseHoveringRect( bb.Min, bb.Max ) && obj.time > 0.3f );
    obj.col =  col_anim( GetColorU32( ImGuiCol_TextDisabled ), GetColorU32( ImGuiCol_TextHovered ), obj.anim );

    add_text( icons, 13, bb.Min, obj.col, question_line );

    GetForegroundDrawList( )->AddRectFilled( { bb.Min.x, bb.Max.y + SCALE( 8 ) }, ImVec2{ bb.Min.x + GImGui->Style.FramePadding.x * 2, bb.Max.y + SCALE( 8 ) + GImGui->Style.FramePadding.y * 2 } + text_size( font, 13, text ), GetColorU32( ImGuiCol_FrameBg, obj.anim ), GImGui->Style.FrameRounding );
    GetForegroundDrawList( )->AddRect( { bb.Min.x, bb.Max.y + SCALE( 8 ) }, ImVec2{ bb.Min.x + GImGui->Style.FramePadding.x * 2, bb.Max.y + SCALE( 8 ) + GImGui->Style.FramePadding.y * 2 } + text_size( font, 13, text ), GetColorU32( ImGuiCol_Border, obj.anim ), GImGui->Style.FrameRounding );
    GetForegroundDrawList( )->AddText( { bb.Min.x + GImGui->Style.FramePadding.x, bb.Max.y + SCALE( 8 ) + GImGui->Style.FramePadding.y }, GetColorU32( ImGuiCol_Text, obj.anim ), text );
}

bool ui::combo( const char* label, int* v, std::vector< const char* > items ) {
    bool result = false;

    combo_ex( label, items[*v], [&]( ) {
        for ( int i = 0; i < items.size( ); ++i ) {
            if ( selectable( items[i], i == *v, { GetWindowWidth( ), GImGui->FontSize } ) ) {
                *v = i;
                result = true;
            }
        }
    }, true );

    return result;
}

void dl::window::draw_bg( ImColor col, float rounding, ImDrawFlags flags ) {
    GetWindowDrawList( )->AddRectFilled( GetWindowPos( ), GetWindowPos( ) + GetWindowSize( ), col, rounding, flags );
}

void dl::window::border( int bord, ImColor col ) {
    if ( bord & border_right ) {
        GetWindowDrawList( )->AddRectFilled( GetWindowPos( ) + ImVec2{ GetWindowWidth( ) - 1, 0 }, GetWindowPos( ) + GetWindowSize( ), col );
    }

    if ( bord & border_top ) {
        GetWindowDrawList( )->AddRectFilled( GetWindowPos( ), GetWindowPos( ) + ImVec2{ GetWindowWidth( ), 1 }, col );
    }

    if ( bord & border_left ) {
        GetWindowDrawList( )->AddRectFilled( GetWindowPos( ), GetWindowPos( ) + ImVec2{ 1, GetWindowHeight( ) }, col );
    }

    if ( bord & border_bottom ) {
        GetWindowDrawList( )->AddRectFilled( GetWindowPos( ) + ImVec2{ 0, GetWindowHeight( ) - 1 }, GetWindowPos( ) + GetWindowSize( ), col );
    }
}

void ui::styles( ) {
    auto &style = GImGui->Style;

    style.WindowRounding = SCALE( 8 );
    style.WindowPadding = ImVec2{ 0, 0 };
    style.WindowBorderSize = 0;

    style.FrameRounding = SCALE( 3 );
    style.FramePadding = SCALE( 8, 5 );
    style.FrameBorderSize = 0;

    style.PopupRounding = SCALE( 3 );
    style.PopupBorderSize = 0;

    style.ChildRounding = SCALE( 6 );
    style.ChildBorderSize = 1;

    style.ItemSpacing = SCALE( 14, 21 );
    style.ItemInnerSpacing = SCALE( 8, 8 );

    style.ScrollbarRounding = SCALE( 4 );
    style.ScrollbarSize = SCALE( 4 );
    style.WindowMinSize = ImVec2{ 1, 1 };
}

void ui::colors( ) {
    ImVec4* colors = GImGui->Style.Colors;

    colors[ImGuiCol_Scheme]                 = ImColor( menu_col[0], menu_col[1], menu_col[2], menu_col[3] );

    colors[ImGuiCol_Text]                   = ImColor( 228, 228, 230 );
    colors[ImGuiCol_TextHovered]            = ImColor( 122, 122, 122, int( 0.6f * 255 ) );
    colors[ImGuiCol_TextDisabled]           = ImColor( 122, 122, 122 );

    colors[ImGuiCol_WindowBg]               = ImColor( 26, 26, 26 );
    colors[ImGuiCol_ChildBg]                = ImColor( 27, 27, 27 );
    colors[ImGuiCol_FrameBg]                = ImColor( 17, 17, 17 );
    colors[ImGuiCol_FrameBgHovered]         = ImColor( 22, 22, 22 );
    colors[ImGuiCol_PopupBg]                = colors[ImGuiCol_FrameBg];
    colors[ImGuiCol_Border]                 = ImColor( 36, 36, 36 );
    colors[ImGuiCol_CheckMark]              = ImColor( 255, 255, 255 );
    colors[ImGuiCol_SliderGrab]             = ImColor( 255, 255, 255 );
    colors[ImGuiCol_TextSelectedBg]         = ImColor( colors[ImGuiCol_Scheme].x, colors[ImGuiCol_Scheme].y, colors[ImGuiCol_Scheme].z, 0.07f );
    colors[ImGuiCol_Tab]                    = ImColor( 29, 30, 37 );

    colors[ImGuiCol_Button]                 = colors[ImGuiCol_FrameBg];
    colors[ImGuiCol_ButtonHovered]          = colors[ImGuiCol_Scheme] / 1.15f;
    colors[ImGuiCol_ButtonActive]           = colors[ImGuiCol_Scheme] / 1.35f;

    colors[ImGuiCol_ScrollbarGrab]          = colors[ImGuiCol_Scheme];
    colors[ImGuiCol_ScrollbarGrabHovered]   = colors[ImGuiCol_Scheme];
    colors[ImGuiCol_ScrollbarGrabActive]    = colors[ImGuiCol_Scheme];
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_Separator]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_WindowShadow]           = ImColor{ 0, 0, 0, 0 };
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.0f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
}