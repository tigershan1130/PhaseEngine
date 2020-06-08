//--------------------------------------------------------------------------------------
// File: Text.h
//
// Text rendering
//
// Modified from DirectX SDK
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Manages the insertion point when drawing text
	//--------------------------------------------------------------------------------------
	class CDXUTTextHelper
	{
	public:
		CDXUTTextHelper( ID3DX10Font* pFont10, ID3DX10Sprite* pSprite10, int nLineHeight = 15 );
		~CDXUTTextHelper();

		void    Init(ID3DX10Font* pFont10 = NULL, ID3DX10Sprite* pSprite10 = NULL, int nLineHeight = 15 );

		void    SetInsertionPos( int x, int y )
		{
			m_pt.x = x; m_pt.y = y;
		}
		void    SetForegroundColor( D3DXCOLOR clr )
		{
			m_clr = clr;
		}

		void    Begin();
		HRESULT DrawFormattedTextLine( const WCHAR* strMsg, ... );
		HRESULT DrawTextLine( const WCHAR* strMsg );
		HRESULT DrawTextLine( const char* strMsg );
		HRESULT DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg, ... );
		HRESULT DrawTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg );
		void    End();

	protected:
		ID3DX10Font* m_pFont10;
		ID3DX10Sprite* m_pSprite10;
		D3DXCOLOR m_clr;
		POINT m_pt;
		int m_nLineHeight;

		ID3D10BlendState* m_pFontBlendState10;
	};
}