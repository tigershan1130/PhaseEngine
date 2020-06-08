//--------------------------------------------------------------------------------------
// File: Common States.fxh
//
// Contains all global rendering states
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Blend States
//--------------------------------------------------------------------------------------

// Depth only
BlendState NoColorWritesBS
{
	RenderTargetWriteMask[0] = 0;
};

// Additive blending
BlendState AdditiveBS
{
    BlendEnable[0] = TRUE;
    SrcBlend = ONE;
    DestBlend = ONE;
};

// Additive blending for 2 targets
BlendState Additive2BS
{
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    SrcBlend = ONE;
    DestBlend = ONE;
};

// Alpha blending
BlendState BlendBS
{
    BlendEnable[0] = TRUE;
    SrcBlend = ONE;
    DestBlend = SRC_ALPHA;
};



// No blending
BlendState DefaultBS
{
    BlendEnable[0] = FALSE;
};


//--------------------------------------------------------------------------------------
// DepthStencil States
//--------------------------------------------------------------------------------------

// No depth buffers
DepthStencilState DisableDepthDS
{
	DepthEnable = FALSE;
};

// No depth writes
DepthStencilState DisableDepthWritesDS
{
	DepthEnable = TRUE;
	DepthFunc = LESS_EQUAL;
	DepthWriteMask = ZERO;
};


// Enable depth buffers
DepthStencilState DefaultDS
{
	DepthEnable = TRUE;
	DepthFunc = LESS_EQUAL;
};

// Enable depth buffers with scencil writes
DepthStencilState StencilFillDS
{
	DepthEnable = TRUE;
	DepthFunc = LESS_EQUAL;

    // Setup stencil states
    StencilEnable = TRUE;
    FrontFaceStencilFunc = ALWAYS;
    FrontFaceStencilPass = INCR_SAT;
    BackFaceStencilFunc = ALWAYS;
    BackFaceStencilPass = INCR_SAT;
};

// Disable depth but enable stencil test
DepthStencilState StencilTestDS
{
	DepthEnable = false;
    DepthWriteMask = ZERO;
    DepthFunc = LESS_EQUAL;
    
    // Setup stencil states
    StencilEnable = TRUE;
    StencilWriteMask = 0x00;
      
    FrontFaceStencilFunc = NOT_EQUAL;
    FrontFaceStencilPass = KEEP;
    FrontFaceStencilFail = ZERO;
      
    BackFaceStencilFunc = NOT_EQUAL;
    BackFaceStencilPass = KEEP;
    BackFaceStencilFail = ZERO;
};


//--------------------------------------------------------------------------------------
// Rasterizer States
//--------------------------------------------------------------------------------------

// Cull frontfaces (draw backfaces)
RasterizerState FrontCullRS
{
	CullMode = FRONT;
};

// Disable culling
RasterizerState NoCullRS
{
	CullMode = NONE;
};

// Wireframe
RasterizerState WireframeRS
{
	CullMode = NONE;
	FillMode = WIREFRAME;
};

// Standard
RasterizerState DefaultRS
{
};

// MSAA
RasterizerState MultisampleRS
{
	MultisampleEnable = TRUE;
};

// Cull frontfaces (draw backfaces) with MSAA
RasterizerState MultisampleFrontCullRS
{
	CullMode = FRONT;
	MultisampleEnable = TRUE;
};