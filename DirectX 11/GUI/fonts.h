#pragma once
#include "font.h"

class c_font {
private:
	unsigned char* data;
	size_t data_size;
	std::unordered_map< float, ImFont* > fonts;
	ImWchar* ranges;
public:
	std::vector< float > should_init = { };

	void init( std::vector< float > sizes, bool add = true ) {
		auto config = ImFontConfig( );
		config.FontDataOwnedByAtlas = false;

		for ( auto& sz : sizes ) {
			if ( add )
				should_init.emplace_back( sz );

			fonts.insert( { sz, ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( data, data_size, sz * dpi_scale, &config, ranges ) } );
		}
	}

	ImFont* get( float sz ) {
		auto it = fonts.find( sz );

		if ( it == fonts.end( ) ) {
			if ( std::find( should_init.begin( ), should_init.end( ), sz ) == should_init.end( ) )
				should_init.emplace_back( sz );

			return nullptr;
		}

		return it->second;
	}

	void set_data( unsigned char* bytes, size_t size ) {
		delete[] data;

        data = new unsigned char[size];
        std::memcpy(data, bytes, size);

        data_size = size;
	}

	void set_ranges( const ImWchar* _ranges ) {
		ranges = ( ImWchar* )_ranges;
	}

	auto& get_fonts( ) {
		return fonts;
	}
};