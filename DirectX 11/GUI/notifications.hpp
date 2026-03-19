#pragma once

namespace notify {
	enum notify_status {
		notify_success,
		notify_error,
		notify_info,
	};

	struct c_notify {
		std::string message;
		notify_status status;
		float duration = 3.f;
		float time = 0.f;
		ImVec2 pos{ 0, 0 };
		float fade_time = 0.25f;
	};

	inline std::vector< c_notify > notifications;

	inline void erase_notifies( ) {
		notifications.erase(
			std::remove_if( notifications.begin( ), notifications.end( ), []( const c_notify& n ) {
				return n.time >= n.duration;
			} ), 
			notifications.end( )
		);
	}

	inline void add( const std::string& message, notify_status status, float duration = 3.f ) {
		notifications.emplace_back( c_notify{ message, status, duration } );
	}

	inline void draw( ) {
		auto draw_list = GetBackgroundDrawList( );

		float offset = 0.f;
		for ( int i = 0; i < notifications.size( ); ++i ) {
			auto& n = notifications[i];
			float alpha = n.time <= n.fade_time ? n.time / n.fade_time : n.time >= n.duration - n.fade_time ? ( n.duration - n.time ) / n.fade_time : 1.f;

			const char* titles[] {
				"success",
				"error",
				"info",
			};

			const char* n_icons[] = {
				check_circle_fill,
				warning_fill,
				information_fill,
			};

			ImColor colors[] {
				{ 164, 188, 167, int( 255 * alpha ) },
				{ 188, 164, 178, int( 255 * alpha ) },
				{ 164, 171, 188, int( 255 * alpha ) },
			};

			ImVec2 size{ ImMax( CalcTextSize( n.message.c_str( ) ).x, CalcTextSize( titles[n.status] ).x + 24 ) + 28, GImGui->FontSize * 2 + 38 };

			if ( n.pos.x == 0 ) n.pos = GetIO( ).DisplaySize - ImVec2{ 0, 20 + offset + size.y };

			n.pos.x = ImLerp( n.pos.x, GetIO( ).DisplaySize.x - 20 - size.x, GetIO( ).DeltaTime * 14 );
			n.pos.y = ImLerp( n.pos.y, GetIO( ).DisplaySize.y - 20 - offset - size.y, GetIO( ).DeltaTime * 14 );

			draw_list->AddRectFilled( n.pos, n.pos + size, GetColorU32( ImGuiCol_WindowBg, alpha ), 3 );

			draw_list->AddText( fonts[icons].get( 14 ), 14, n.pos + ImVec2{ 14, 14 }, colors[n.status], n_icons[n.status] );
			draw_list->AddText( n.pos + ImVec2{ 38, 16 }, GetColorU32( ImGuiCol_Text, alpha ), titles[n.status] );
			draw_list->AddText( n.pos + ImVec2{ 14, 24 + GImGui->FontSize }, GetColorU32( ImGuiCol_TextDisabled, alpha ), n.message.c_str( ) );

			n.time += 1.f / GetIO( ).Framerate;
			offset += size.y + 12;
		}

		erase_notifies( );
	}
}