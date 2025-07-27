#pragma once

#include <openxr/openxr.h>
#include <vector>
#include <string>
#include <functional>

#include "Input/ActionInfo.h"
#include "Input/ActionSetInfo.h"
#include "Input/InteractionProfileBinding.h"

class OpenXRInputMgr
{
public:
    static void Shutdown();
    static void Tick(XrTime predictedTime, XrSpace referenceSpace);
    
    static bool GetSelectDown(int handIndex);  // Button just pressed
    static bool GetSelect(int handIndex);      // Button held down
    static bool GetSelectUp(int handIndex);    // Button just released
    static XrPosef GetHandPose(int handIndex, bool* isActive = nullptr);
    
    static void TriggerHapticFeedback(int handIndex, float amplitude = 0.5f, XrDuration duration = 100000000);
    
    static void CreateActionSet(const std::string& actionSetName, const std::string& localizedName, uint32_t priority = 0);
    static void DestroyActionSet();

    static void SetupActions();

    static void SetupBindings();
    static void AttachActionSet();

    static void CreateHandPoseActionSpace();
    
private:
    struct HandState {
        bool currentSelectPressed = false;
        bool lastSelectPressed = false;
        XrPosef pose = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
        bool poseActive = false;
    };
    
    static HandState m_HandStates[2];  // Left and Right hand
    
    // 改为单个action，不再使用数组
    static XrAction m_HandPoseAction;
    static XrAction m_SelectAction;
    static XrAction m_HapticAction;
    
    static XrSpace m_HandSpaces[2];
    
    static void UpdateHandStates(XrTime predictedTime, XrSpace referenceSpace);
    
    
    static XrAction CreateAction(const std::string& actionName, const std::string& localizedName, 
                                XrActionType actionType, const std::vector<std::string>& subactionPaths = {});
    
    static void AddBindingForProfile(const std::string& interactionProfilePath, 
                              const std::vector<std::pair<XrAction, std::string>>& actionBindings);
    
    
    static XrSpace CreateActionSpace(XrAction poseAction, const std::string& subactionPath = "");
    static void DestroyActionSpaces();
    
    static void SyncActions();
    static bool GetActionStateBoolean(XrAction action, const std::string& subactionPath = "", bool* changedSinceLastSync = nullptr);
    static float GetActionStateFloat(XrAction action, const std::string& subactionPath = "", bool* changedSinceLastSync = nullptr);
    static XrPosef GetActionStatePose(XrAction poseAction, XrSpace actionSpace, XrSpace referenceSpace, XrTime predictedTime, bool* isActive = nullptr);
    
    static void ApplyHapticFeedback(XrAction hapticAction, const std::string& subactionPath, 
                                   float amplitude, XrDuration duration = XR_MIN_HAPTIC_DURATION, 
                                   float frequency = XR_FREQUENCY_UNSPECIFIED);
    
    static void GetCurrentInteractionProfile(const std::string& subactionPath, std::string& profilePath);

    static ActionSetInfo m_ActionSet;
    static std::vector<InteractionProfileBinding> m_InteractionProfileBindings;
    static std::vector<XrSpace> m_ActionSpaces;
    
    static constexpr const char* HAND_LEFT_PATH = "/user/hand/left";
    static constexpr const char* HAND_RIGHT_PATH = "/user/hand/right";
    static constexpr const char* SIMPLE_CONTROLLER_PROFILE = "/interaction_profiles/khr/simple_controller";
};
