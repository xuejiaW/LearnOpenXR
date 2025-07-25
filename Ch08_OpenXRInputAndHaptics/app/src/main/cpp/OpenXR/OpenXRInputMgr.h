﻿#pragma once

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
    // High-level interface for game development
    static void Initialize();
    static void Shutdown();
    static void Tick(XrTime predictedTime, XrSpace referenceSpace);
    
    // Input state queries
    static bool GetSelectDown(int handIndex);  // Button just pressed
    static bool GetSelect(int handIndex);      // Button held down
    static bool GetSelectUp(int handIndex);    // Button just released
    static XrPosef GetHandPose(int handIndex, bool* isActive = nullptr);
    
    // Haptic feedback
    static void TriggerHapticFeedback(int handIndex, float amplitude = 0.5f, XrDuration duration = 100000000);
    
    // Utility functions (remain public)
    static const char* GetHandPath(int handIndex);
    static const char* GetSimpleControllerProfilePath();
    static const char* GetTouchControllerProfilePath();
    
    static void OnInteractionProfileChanged();

private:
    // Internal state tracking
    struct HandState {
        bool currentSelectPressed = false;
        bool lastSelectPressed = false;
        XrPosef pose = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
        bool poseActive = false;
    };
    
    static HandState m_HandStates[2];  // Left and Right hand
    
    // Input actions
    static XrAction m_HandPoseActions[2];
    static XrAction m_SelectActions[2];
    static XrAction m_HapticActions[2];
    
    // Action spaces
    static XrSpace m_HandSpaces[2];
    
    // Internal initialization and setup
    static void SetupActions();
    static void SetupBindings();
    static void UpdateHandStates(XrTime predictedTime, XrSpace referenceSpace);
    
    // Low-level OpenXR functions (now private)
    static void CreateActionSet(const std::string& actionSetName, const std::string& localizedName, uint32_t priority = 0);
    static void DestroyActionSets();
    
    static XrAction CreateAction(const std::string& actionName, const std::string& localizedName, 
                                XrActionType actionType, const std::vector<std::string>& subactionPaths = {});
    
    static void SuggestBinding(const std::string& interactionProfilePath, 
                              const std::vector<std::pair<XrAction, std::string>>& actionBindings);
    static void SuggestAllBindings();
    
    static void AttachActionSets();
    
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

    // Static member variables
    static std::vector<ActionSetInfo> m_ActionSets;
    static std::vector<InteractionProfileBinding> m_InteractionProfileBindings;
    static std::vector<XrSpace> m_ActionSpaces;
    
    static constexpr const char* HAND_LEFT_PATH = "/user/hand/left";
    static constexpr const char* HAND_RIGHT_PATH = "/user/hand/right";
    static constexpr const char* SIMPLE_CONTROLLER_PROFILE = "/interaction_profiles/khr/simple_controller";
    static constexpr const char* TOUCH_CONTROLLER_PROFILE = "/interaction_profiles/oculus/touch_controller";
};
