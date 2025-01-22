#include <stdint.h>

#ifndef CLAY_CLICK_HANDLER_HEADER
#define CLAY_CLICK_HANDLER_HEADER

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)

#define CLICK_IS(name, phase_token) clickHandler_##name##_clickPhase == CLICK_PHASE_##phase_token
#define CLICK_SET(name, phase_token) clickHandler_##name##_clickPhase = CLICK_PHASE_##phase_token
#define POINTER_IS(data_token) pointerInfo.state == CLAY_POINTER_DATA_##data_token
#define ON_HOVER_PARAMS Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData
#define ON_HOVER_ARGS elementId, pointerInfo, userData

#define CLAY_CLICK_HANDLER(name, __VA_ARGS__1, __VA_ARGS__2, __VA_ARGS__3, __VA_ARGS__4, __VA_ARGS__5) \
	float clickHandler_##name##_beforeClickWaitTime; \
	ClickPhase clickHandler_##name##_clickPhase; \
	Clay_ElementId clickHandler_##name##_elementId; \
	bool clickHandler_##name##_inTouchStart; \
	void handle_##name##_clickInteraction(ON_HOVER_PARAMS) \
	{ \
		if (POINTER_IS(PRESSED_THIS_FRAME) || POINTER_IS(PRESSED)) \
		{ \
			if (!isInterrupted) \
			{ \
				if (pointerStartLock_waitTime > currentTime) \
				{ \
					clickHandler_##name##_inTouchStart = true; \
				} \
				if (!clickHandler_##name##_inTouchStart) \
				{ \
					return; \
				} \
				__VA_ARGS__2 \
				clickHandler_##name##_elementId = elementId; \
				CLICK_SET(name, PRESSED_THIS_FRAME); \
				clickHandler_##name##_beforeClickWaitTime = currentTime + MAX(0.01f, averageDeltaTime * 2); \
			} \
		} \
	} \
	void listen_##name##_click() { \
		static float mousePos2_waitTime; \
		if (isInterrupted) \
		{ \
			__VA_ARGS__3 \
			clickHandler_##name##_inTouchStart = false; \
			CLICK_SET(name, NONE); \
			return; \
		} \
		if (clickHandler_##name##_beforeClickWaitTime > currentTime) \
		{ \
			CLICK_SET(name, PRESSED); \
			return; \
		} \
		if (CLICK_IS(name, PRESSED)) \
		{ \
			if (isPointerDown) \
			{ \
				__VA_ARGS__4 \
				return; \
			} \
			clickHandler_##name##_inTouchStart = false; \
			CLICK_SET(name, CHECK_POS_THIS_FRAME); \
			mousePos2_waitTime = currentTime + MAX(0.01f, averageDeltaTime * 2); \
			return; \
		} \
		if (mousePos2_waitTime > currentTime) \
		{ \
			CLICK_SET(name, CHECK_POS); \
			return; \
		} \
		if (CLICK_IS(name, CHECK_POS)) \
		{ \
			__VA_ARGS__1 \
			CLICK_SET(name, NONE); \
			return; \
		} \
		__VA_ARGS__5 \
	}

typedef enum : uint8_t {
	CLICK_PHASE_NONE,
	CLICK_PHASE_PRESSED_THIS_FRAME,
	CLICK_PHASE_PRESSED,
	CLICK_PHASE_CHECK_POS_THIS_FRAME,
	CLICK_PHASE_CHECK_POS,
} ClickPhase;

#endif // CLAY_CLICK_HANDLER_HEADER




#ifdef CLAY_CLICK_HANDLER_IMPLEMENTATION
#undef CLAY_CLICK_HANDLER_IMPLEMENTATION

bool isPointerCancel;
bool isPointerStart;
bool isPointerDown;
bool isInterrupted;
float pointerStartLock_waitTime;
float currentTime;
float deltaTime;
float averageDeltaTime;

#define N_STEPS_AVERAGE_DELTA_TIME_CYCLE 20
void updateAverageDeltaTime()
{
	static float cycleData[N_STEPS_AVERAGE_DELTA_TIME_CYCLE] = {0};
	static int cycleIndex = 0;
	static bool isPopulated = false;
	float deltaTimeSum = 0;
	if (isPopulated)
	{
		for (int i = 0; i < N_STEPS_AVERAGE_DELTA_TIME_CYCLE; i++)
		{
			deltaTimeSum += cycleData[i];
		}
	}
	else
	{
		for (int i = 0; i <= cycleIndex; i++)
		{
			deltaTimeSum += cycleData[i];
		}
		isPopulated = cycleIndex + 1 == N_STEPS_AVERAGE_DELTA_TIME_CYCLE;
	}
	averageDeltaTime = deltaTimeSum / N_STEPS_AVERAGE_DELTA_TIME_CYCLE;
	cycleData[cycleIndex] = deltaTime;
	cycleIndex = cycleIndex + 1 == N_STEPS_AVERAGE_DELTA_TIME_CYCLE ? 0 : cycleIndex + 1;
}

#endif // CLAY_CLICK_HANDLER_IMPLEMENTATION
