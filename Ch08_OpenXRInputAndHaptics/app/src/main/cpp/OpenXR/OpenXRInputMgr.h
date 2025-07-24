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
    // ActionSet management
    static void CreateActionSet(const std::string& actionSetName, const std::string& localizedName, uint32_t priority = 0);
    static void DestroyActionSets();
    
    // Action management
    static XrAction CreateAction(const std::string& actionName, const std::string& localizedName, 
                                XrActionType actionType, const std::vector<std::string>& subactionPaths = {});
    
    // Interaction profile binding
    static void SuggestBinding(const std::string& interactionProfilePath, 
                              const std::vector<std::pair<XrAction, std::string>>& actionBindings);
    static void SuggestAllBindings();
    
    // Session attachment
    static void AttachActionSets();
    
    // Action space management (for pose actions)
    static XrSpace CreateActionSpace(XrAction poseAction, const std::string& subactionPath = "");
    static void DestroyActionSpaces();
    
    // Input polling
    static void SyncActions();
    static bool GetActionStateBoolean(XrAction action, const std::string& subactionPath = "", bool* changedSinceLastSync = nullptr);
    static float GetActionStateFloat(XrAction action, const std::string& subactionPath = "", bool* changedSinceLastSync = nullptr);
    static XrPosef GetActionStatePose(XrAction poseAction, XrSpace actionSpace, XrSpace referenceSpace, XrTime predictedTime, bool* isActive = nullptr);
    
    // Haptic feedback
    static void ApplyHapticFeedback(XrAction hapticAction, const std::string& subactionPath, 
                                   float amplitude, XrDuration duration = XR_MIN_HAPTIC_DURATION, 
                                   float frequency = XR_FREQUENCY_UNSPECIFIED);
    
    // Interaction profile utilities
    static void GetCurrentInteractionProfile(const std::string& subactionPath, std::string& profilePath);
    
    // Event handling
    static void OnInteractionProfileChanged();

private:
    static std::vector<ActionSetInfo> m_ActionSets;
    static std::vector<InteractionProfileBinding> m_InteractionProfileBindings;
    static std::vector<XrSpace> m_ActionSpaces;
    static bool m_IsAttached;
};
