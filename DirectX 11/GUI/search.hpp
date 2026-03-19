#pragma once

#include <iostream>

using namespace ImGui;

inline char search_buf[16];

struct c_child {
    int tab;
    int subtab;
    const char* label;
    const char* desc;
    std::function< void( ) > content;
    std::vector< const char* > widgets{ };
};

inline std::vector< c_child > childs;

inline void add_child( int tab, int subtab, const char* label, const char* desc, std::function< void( ) > content ) {
    struct s {
        bool init = false;
    }; auto& obj = anim_obj( label, 3433232, s{ } );

    if ( !obj.init && !( GImGui->CurrentItemFlags & ImGuiItemFlags_Search ) ) {
        
        childs.emplace_back( c_child{ tab, subtab, label, desc, content } );

        obj.init = true;
    }
}

inline std::string to_lower( const char* str ) {
    std::string result;

    for (int i = 0; str[i]; ++i) {
        result += std::tolower(str[i]);
    }

    return result;
}

inline void init_search( ) {
    static bool init = false;

    if ( !init ) {
        for ( int i = 0; i < ui::tabs.size( ); ++i ) {
            if ( ui::tabs[i].pages.empty( ) ) continue;

            for ( int v = 0; v < ui::tabs[i].subtabs.size( ); ++v ) {
                if ( ui::tabs[i].pages.size( ) <= v || ui::tabs[i].pages.empty( ) ) continue;

                ui::tabs[i].pages[v]( );
            }
        }

        init = true;
    }
}

inline bool add_widget( const char* label ) {
    if ( childs.empty( ) ) return false;

    struct s {
        bool init = false;
    }; auto& obj = anim_obj( label, 433232, s{ } );

    if ( !obj.init && !( GImGui->CurrentItemFlags & ImGuiItemFlags_Search ) ) {
        childs[childs.size( ) - 1].widgets.emplace_back( label );
        obj.init = true;
    }

    int cur_child = 0;

    while ( cur_child < childs.size( ) - 1 ) {
        bool should_break = false;
        for ( auto& l : childs[cur_child].widgets ) {
            if ( strcmp( label, l ) == 0 ) {
                should_break = true;
                break;
            }
        }

        if ( should_break ) break;
        cur_child++;
    }
    
    return strstr( to_lower( childs[cur_child].label ).c_str( ), to_lower( search_buf ).c_str( ) ) || strstr( to_lower( label ).c_str( ), to_lower( search_buf ).c_str( ) ) || !( GImGui->CurrentItemFlags & ImGuiItemFlags_Search );
}

inline void render_window( ) {
    if ( strlen( search_buf ) == 0 )
        return;

    for ( auto& child : childs ) {
        bool widget_match = false;

        for ( auto& w : child.widgets ) {
            if ( strstr( to_lower( w ).c_str( ), to_lower( search_buf ).c_str( ) ) )
                widget_match = true;
        }

        if ( !widget_match && !strstr( to_lower( child.label ).c_str( ), to_lower( search_buf ).c_str( ) ) )
            continue;

        PushItemFlag( ImGuiItemFlags_Search, true );
        //Text( ui::tabs[child.tab].label );
        if ( !ui::tabs[child.tab].subtabs.empty( ) ) {
            SameLine( 0, 0 );
            Text( ", " );
            SameLine( 0, 0 );
            TextDisabled( ui::tabs[child.tab].subtabs[child.subtab] );
        }
        ui::child( std::string( child.label ).append( "##" ).append( std::to_string( child.tab ) ).c_str( ), child.content, { GetWindowWidth( ) - GImGui->Style.WindowPadding.x * 2, 0 } );
        PopItemFlag( );
        //Spacing( );
    }
}