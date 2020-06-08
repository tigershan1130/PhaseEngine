//--------------------------------------------------------------------------------------
// File: Text.cpp
//
// Text rendering
//
// Modified from DirectX SDK
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Text.h"

namespace Core
{
	CDXUTTextHelper::CDXUTTextHelper( ID3DX10Font* pFont, ID3DX10Sprite* pSprite, int nLineHeight )
	{
		Init( pFont, pSprite, nLineHeight );
	}
	CDXUTTextHelper::~CDXUTTextHelper()
	{
		SAFE_RELEASE( m_pFontBlendState10 );
	}

	//--------------------------------------------------------------------------------------
	void CDXUTTextHelper::Init( ID3DX10Font* pFont10, ID3DX10Sprite* pSprite10,
		int nLineHeight )
	{
		m_pFont10 = pFont10;
		m_pSprite10 = pSprite10;
		m_clr = D3DXCOLOR( 1, 1, 1, 1 );
		m_pt.x = 0;
		m_pt.y = 0;
		m_nLineHeight = nLineHeight;
		m_pFontBlendState10 = NULL;

		// Create a blend state if a sprite is passed in
		if( pSprite10 )
		{
			ID3D10Device* pDev = NULL;
			pSprite10->GetDevice( &pDev );
			if( pDev )
			{
				D3D10_BLEND_DESC StateDesc;
				ZeroMemory( &StateDesc, sizeof( D3D10_BLEND_DESC ) );
				StateDesc.AlphaToCoverageEnable = FALSE;
				StateDesc.BlendEnable[0] = TRUE;
				StateDesc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
				StateDesc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
				StateDesc.BlendOp = D3D10_BLEND_OP_ADD;
				StateDesc.SrcBlendAlpha = D3D10_BLEND_ZERO;
				StateDesc.DestBlendAlpha = D3D10_BLEND_ZERO;
				StateDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
				StateDesc.RenderTargetWriteMask[0] = 0xf;
				pDev->CreateBlendState( &StateDesc, &m_pFontBlendState10 );

				pDev->Release();
			}
		}
	}


	//--------------------------------------------------------------------------------------
	HRESULT CDXUTTextHelper::DrawFormattedTextLine( const WCHAR* strMsg, ... )
	{
		WCHAR strBuffer[512];

		va_list args;
		va_start( args, strMsg );
		swprintf_s( strBuffer, 512, strMsg, args );
		strBuffer[511] = L'\0';
		va_end( args );

		return DrawTextLine( strBuffer );
	}


	//--------------------------------------------------------------------------------------
	HRESULT CDXUTTextHelper::DrawTextLine( const WCHAR* strMsg )
	{
		if( NULL == m_pFont10 )
		{
			LOG( "Text Fail" );
			return E_FAIL;
		}

		HRESULT hr;
		RECT rc;
		SetRect( &rc, m_pt.x, m_pt.y, 0, 0 );
		hr = m_pFont10->DrawText( m_pSprite10, strMsg, -1, &rc, DT_NOCLIP, m_clr );
		if( FAILED( hr ) )
		{
			LOG( "Text Fail" );
			return E_FAIL;
		}

		m_pt.y += m_nLineHeight;

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	HRESULT CDXUTTextHelper::DrawTextLine( const char* strMsg )
	{
		if( NULL == m_pFont10 )
		{
			LOG( "Text Fail" );
			return E_FAIL;
		}

		HRESULT hr;
		RECT rc;
		SetRect( &rc, m_pt.x, m_pt.y, 0, 0 );
		hr = m_pFont10->DrawTextA( m_pSprite10, strMsg, -1, &rc, DT_NOCLIP, m_clr );
		if( FAILED( hr ) )
		{
			LOG( "Text Fail" );
			return E_FAIL;
		}

		m_pt.y += m_nLineHeight;

		return S_OK;
	}

	HRESULT CDXUTTextHelper::DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg, ... )
	{
		WCHAR strBuffer[512];

		va_list args;
		va_start( args, strMsg );
		swprintf_s( strBuffer, 512, strMsg, args );
		strBuffer[511] = L'\0';
		va_end( args );

		return DrawTextLine( rc, dwFlags, strBuffer );
	}


	HRESULT CDXUTTextHelper::DrawTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg )
	{
		if(NULL == m_pFont10 )
		{
			LOG( "Text Fail" );
			return E_FAIL;
		}

		HRESULT hr;
		hr = m_pFont10->DrawText( m_pSprite10, strMsg, -1, &rc, dwFlags, m_clr );
		if( FAILED( hr ) )
		{
			LOG( "Text Fail" );
			return E_FAIL;
		}

		m_pt.y += m_nLineHeight;

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	void CDXUTTextHelper::Begin()
	{
		if( m_pSprite10 )
		{
			D3D10_VIEWPORT VPs[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			UINT cVPs = 1;
			ID3D10Device* pd3dDevice = NULL;
			m_pSprite10->GetDevice( &pd3dDevice );
			if( pd3dDevice )
			{
				// Set projection
				pd3dDevice->RSGetViewports( &cVPs, VPs );
				D3DXMATRIXA16 matProjection;
				D3DXMatrixOrthoOffCenterLH( &matProjection, ( FLOAT )VPs[0].TopLeftX, ( FLOAT )
					( VPs[0].TopLeftX + VPs[0].Width ), ( FLOAT )VPs[0].TopLeftY, ( FLOAT )
					( VPs[0].TopLeftY + VPs[0].Height ), 0.1f, 10 );
				m_pSprite10->SetProjectionTransform( &matProjection );

				m_pSprite10->Begin( D3DX10_SPRITE_SORT_TEXTURE );
				SAFE_RELEASE( pd3dDevice );
			}
		}


	}
	void CDXUTTextHelper::End()
	{
		if( m_pSprite10 )
		{
			FLOAT OriginalBlendFactor[4];
			UINT OriginalSampleMask = 0;
			ID3D10BlendState* pOriginalBlendState10 = NULL;
			ID3D10Device* pd3dDevice = NULL;

			m_pSprite10->GetDevice( &pd3dDevice );
			if( pd3dDevice )
			{
				// Get the old blend state and set the new one
				pd3dDevice->OMGetBlendState( &pOriginalBlendState10, OriginalBlendFactor, &OriginalSampleMask );
				if( m_pFontBlendState10 )
				{
					FLOAT NewBlendFactor[4] = {0,0,0,0};
					pd3dDevice->OMSetBlendState( m_pFontBlendState10, NewBlendFactor, 0xffffffff );
				}
			}

			m_pSprite10->End();

			// Reset the original blend state
			if( pd3dDevice && pOriginalBlendState10 )
			{
				pd3dDevice->OMSetBlendState( pOriginalBlendState10, OriginalBlendFactor, OriginalSampleMask );
			}
			SAFE_RELEASE( pOriginalBlendState10 );
			SAFE_RELEASE( pd3dDevice );
		}
	}
}