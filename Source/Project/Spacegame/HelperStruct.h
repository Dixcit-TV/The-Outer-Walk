#pragma once

enum class GameStates
{
	GAME_START, IN_GAME_MENU, BACK_TO_MAIN, RESUME, PLAYING, TELEPORT, GAME_END
};

struct NoiseParameter
{
	static const int PARAMCOUNT = 7;
	union
	{
		float pData[PARAMCOUNT] = {};
		struct
		{
			float NoiseHeight{ 0 };
			float NoiseCount{ 1 };
			float Frequency{};
			float FrequencyMultiplier{ 1 };
			float Amplitude{};
			float Offset{};

			float VerticalShift{};
		} data;
	};
};

struct ColorSettings
{
	DirectX::XMFLOAT3 UnderwaterGroundColor{};
	DirectX::XMFLOAT3 ShoreColor{};
	DirectX::XMFLOAT3 PlaneColorLow{};
	DirectX::XMFLOAT3 PlaneColorHigh{};
	DirectX::XMFLOAT3 HeightsColor{};

	DirectX::XMFLOAT3 ShoreColorSettings{}; //DirectX::XMFLOAT3{ MaxHeight, blendHeightStart, blendHeightEnd }
	DirectX::XMFLOAT3 PlaneColorSettings{}; //DirectX::XMFLOAT3{ MaxHeight, blendHeightStart, blendHeightEnd }
};