///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENGINEGUIDS_H
#define INCLUDED_ENGINEGUIDS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

// {48DD463F-E619-417a-9776-AB402758E99A}
DEFINE_GUID(IID_ISimClient, 
0x48dd463f, 0xe619, 0x417a, 0x97, 0x76, 0xab, 0x40, 0x27, 0x58, 0xe9, 0x9a);

// {9B8CD9ED-1CA1-40cf-8C67-06A780B49439}
DEFINE_GUID(IID_ISim, 
0x9b8cd9ed, 0x1ca1, 0x40cf, 0x8c, 0x67, 0x6, 0xa7, 0x80, 0xb4, 0x94, 0x39);

// {49698F40-3EDB-44d9-9906-3D3514061897}
DEFINE_GUID(IID_IInput, 
0x49698f40, 0x3edb, 0x44d9, 0x99, 0x6, 0x3d, 0x35, 0x14, 0x6, 0x18, 0x97);

// {A12C2385-265E-49b6-A421-361224CD4F5D}
DEFINE_GUID(IID_IInputListener, 
0xa12c2385, 0x265e, 0x49b6, 0xa4, 0x21, 0x36, 0x12, 0x24, 0xcd, 0x4f, 0x5d);

// {7EABF652-F7D6-42c1-8C5F-AE8564FCC6C6}
DEFINE_GUID(IID_IScriptable,
0x7eabf652, 0xf7d6, 0x42c1, 0x8c, 0x5f, 0xae, 0x85, 0x64, 0xfc, 0xc6, 0xc6);

// {C9F12147-DA9E-45e4-B582-BBA407427403}
DEFINE_GUID(IID_IScriptableFactory, 
0xc9f12147, 0xda9e, 0x45e4, 0xb5, 0x82, 0xbb, 0xa4, 0x7, 0x42, 0x74, 0x3);

// {55819C5F-DFA2-4988-B49D-B772C2DD7532}
DEFINE_GUID(IID_IScriptInterpreter, 
0x55819c5f, 0xdfa2, 0x4988, 0xb4, 0x9d, 0xb7, 0x72, 0xc2, 0xdd, 0x75, 0x32);

// {85E6F9DB-639F-411d-B365-86A8FBD1ACBF}
DEFINE_GUID(IID_IEntity, 
0x85e6f9db, 0x639f, 0x411d, 0xb3, 0x65, 0x86, 0xa8, 0xfb, 0xd1, 0xac, 0xbf);

// {D1A48ABA-7DB7-4fb7-96E3-72F79DFABA99}
DEFINE_GUID(IID_IEntityComponent, 
0xd1a48aba, 0x7db7, 0x4fb7, 0x96, 0xe3, 0x72, 0xf7, 0x9d, 0xfa, 0xba, 0x99);

// {AF68F8F0-EFA5-49c6-AA91-C4E21BAF6D14}
DEFINE_GUID(IID_IEntityRenderComponent, 
0xaf68f8f0, 0xefa5, 0x49c6, 0xaa, 0x91, 0xc4, 0xe2, 0x1b, 0xaf, 0x6d, 0x14);

// {612C76A2-151D-4322-9687-3374463BF7BA}
DEFINE_GUID(IID_IEntitySpawnComponent, 
0x612c76a2, 0x151d, 0x4322, 0x96, 0x87, 0x33, 0x74, 0x46, 0x3b, 0xf7, 0xba);

// {FC22B764-3FBA-43da-97B0-56857D0A77E9}
DEFINE_GUID(IID_IEntityEnum, 
0xfc22b764, 0x3fba, 0x43da, 0x97, 0xb0, 0x56, 0x85, 0x7d, 0xa, 0x77, 0xe9);

// {92DB7247-E01C-4935-B35C-EB233295A4BE}
DEFINE_GUID(IID_IEntityManager, 
0x92db7247, 0xe01c, 0x4935, 0xb3, 0x5c, 0xeb, 0x23, 0x32, 0x95, 0xa4, 0xbe);

// {1EC6DB1A-C833-4b68-8705-D1A9FB5CC8D3}
DEFINE_GUID(IID_IEntityManagerListener, 
0x1ec6db1a, 0xc833, 0x4b68, 0x87, 0x5, 0xd1, 0xa9, 0xfb, 0x5c, 0xc8, 0xd3);

// {A7D128C5-74AD-4f46-848B-33A9371978A6}
DEFINE_GUID(IID_ISaveLoadManager, 
0xa7d128c5, 0x74ad, 0x4f46, 0x84, 0x8b, 0x33, 0xa9, 0x37, 0x19, 0x78, 0xa6);

// {D67CE569-DB15-453b-B616-03D63572C6C6}
DEFINE_GUID(IID_ISaveLoadParticipant, 
0xd67ce569, 0xdb15, 0x453b, 0xb6, 0x16, 0x3, 0xd6, 0x35, 0x72, 0xc6, 0xc6);

// {A5C83344-7BFC-4470-8952-3D0FBAD13F05}
DEFINE_GUID(IID_ITerrainRenderer, 
0xa5c83344, 0x7bfc, 0x4470, 0x89, 0x52, 0x3d, 0xf, 0xba, 0xd1, 0x3f, 0x5);

// {F8DFB0B9-1EA3-44a9-AE3E-B4448E339254}
DEFINE_GUID(IID_IEnumTerrainQuads, 
0xf8dfb0b9, 0x1ea3, 0x44a9, 0xae, 0x3e, 0xb4, 0x44, 0x8e, 0x33, 0x92, 0x54);

// {2C77E8F2-927A-4b6a-B8F2-E0EF8BE03CBD}
DEFINE_GUID(IID_ITerrainModel, 
0x2c77e8f2, 0x927a, 0x4b6a, 0xb8, 0xf2, 0xe0, 0xef, 0x8b, 0xe0, 0x3c, 0xbd);

// {2B0FB4C1-1455-4f57-9AE2-51BFEB615EBF}
DEFINE_GUID(IID_ITerrainModelListener, 
0x2b0fb4c1, 0x1455, 0x4f57, 0x9a, 0xe2, 0x51, 0xbf, 0xeb, 0x61, 0x5e, 0xbf);

// {918AF8C2-2B08-45a6-A86F-FB6E1345AAD3}
DEFINE_GUID(IID_ITerrainTileSet, 
0x918af8c2, 0x2b08, 0x45a6, 0xa8, 0x6f, 0xfb, 0x6e, 0x13, 0x45, 0xaa, 0xd3);

// {B92B9D13-FA3C-48a7-B93A-A466FB8E2C4D}
DEFINE_GUID(IID_IHeightMap, 
0xb92b9d13, 0xfa3c, 0x48a7, 0xb9, 0x3a, 0xa4, 0x66, 0xfb, 0x8e, 0x2c, 0x4d);

// {CB95ECA7-B3B9-48fe-9B87-FE042201215A}
DEFINE_GUID(IID_ICamera, 
0xcb95eca7, 0xb3b9, 0x48fe, 0x9b, 0x87, 0xfe, 0x4, 0x22, 0x1, 0x21, 0x5a);

// {1D9FA523-EA63-4793-A0B2-B7B624521CAA}
DEFINE_GUID(IID_ICameraControl, 
0x1d9fa523, 0xea63, 0x4793, 0xa0, 0xb2, 0xb7, 0xb6, 0x24, 0x52, 0x1c, 0xaa);

// {062A362E-30FA-4459-BC04-36ED29EE5A85}
DEFINE_GUID(IID_ISoundManager, 
0x62a362e, 0x30fa, 0x4459, 0xbc, 0x4, 0x36, 0xed, 0x29, 0xee, 0x5a, 0x85);

// {B1F78800-CEC7-4df4-9AF6-25D8AC296EC2}
DEFINE_GUID(IID_IRenderer, 
0xb1f78800, 0xcec7, 0x4df4, 0x9a, 0xf6, 0x25, 0xd8, 0xac, 0x29, 0x6e, 0xc2);

// {96AB8D29-43F8-413b-812D-18CC805B6ABE}
DEFINE_GUID(IID_IModelAnimationController, 
0x96ab8d29, 0x43f8, 0x413b, 0x81, 0x2d, 0x18, 0xcc, 0x80, 0x5b, 0x6a, 0xbe);

// {FC4D1E2C-9490-4825-A5DC-5E8E92D5BC41}
DEFINE_GUID(IID_IModelKeyFrameInterpolator, 
0xfc4d1e2c, 0x9490, 0x4825, 0xa5, 0xdc, 0x5e, 0x8e, 0x92, 0xd5, 0xbc, 0x41);

// {74189271-A55B-46e9-BD57-10005302DF0B}
DEFINE_GUID(IID_IModelAnimation, 
0x74189271, 0xa55b, 0x46e9, 0xbd, 0x57, 0x10, 0x0, 0x53, 0x2, 0xdf, 0xb);

// {815EB67E-39BE-4190-9188-2CDD41233140}
DEFINE_GUID(IID_IModelSkeleton, 
0x815eb67e, 0x39be, 0x4190, 0x91, 0x88, 0x2c, 0xdd, 0x41, 0x23, 0x31, 0x40);

// {C82434F3-7467-4184-8418-AECC898D37AF}
DEFINE_GUID(SAVELOADID_MapProperties, 
0xc82434f3, 0x7467, 0x4184, 0x84, 0x18, 0xae, 0xcc, 0x89, 0x8d, 0x37, 0xaf);

// {79C13C86-71E5-455e-B763-927F31B74B82}
DEFINE_GUID(SAVELOADID_TerrainModel, 
0x79c13c86, 0x71e5, 0x455e, 0xb7, 0x63, 0x92, 0x7f, 0x31, 0xb7, 0x4b, 0x82);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENGINEGUIDS_H
