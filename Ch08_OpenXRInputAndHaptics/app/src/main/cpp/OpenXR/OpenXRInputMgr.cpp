#include "OpenXRInputMgr.h"

#include <DebugOutput.h>
#include <OpenXRHelper.h>
#include <XRPathUtils.h>
#include "OpenXRCoreMgr.h"

ActionSetInfo OpenXRInputMgr::m_ActionSet{};
std::vector<InteractionProfileBinding> OpenXRInputMgr::m_InteractionProfileBindings{};

ControllerState OpenXRInputMgr::controllerStates[2] = {};
XrAction OpenXRInputMgr::m_HandPoseAction = XR_NULL_HANDLE;
XrAction OpenXRInputMgr::m_SelectAction = XR_NULL_HANDLE;
XrAction OpenXRInputMgr::m_HapticAction = XR_NULL_HANDLE;
XrSpace OpenXRInputMgr::m_HandSpaces[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};

void OpenXRInputMgr::Shutdown()
{
    DestroyActionSpaces();
    DestroyActionSet();

    for (int i = 0; i < 2; ++i)
    {
        controllerStates[i] = {};
    }

    XR_TUT_LOG("OpenXRInputMgr shutdown completed");
}

void OpenXRInputMgr::Tick(XrTime predictedTime, XrSpace referenceSpace)
{
    SyncActions();
    UpdateControllerStates(predictedTime, referenceSpace);
}

void OpenXRInputMgr::TriggerHapticFeedback(int handIndex, float amplitude, XrDuration duration)
{
    ApplyHapticFeedback(m_HapticAction, handIndex == 0 ? HAND_LEFT_PATH : HAND_RIGHT_PATH, amplitude, duration);
}

void OpenXRInputMgr::SetupActions()
{
    std::vector<std::string> bothHandsSubactions = {HAND_LEFT_PATH, HAND_RIGHT_PATH};

    m_HandPoseAction = CreateAction("hand_pose", "Hand Pose", XR_ACTION_TYPE_POSE_INPUT, bothHandsSubactions);
    m_SelectAction = CreateAction("trigger_select", "Trigger Select", XR_ACTION_TYPE_BOOLEAN_INPUT, bothHandsSubactions);
    m_HapticAction = CreateAction("haptic_feedback", "Haptic Feedback", XR_ACTION_TYPE_VIBRATION_OUTPUT, bothHandsSubactions);
}

void OpenXRInputMgr::CreateHandPoseActionSpace()
{
    m_HandSpaces[0] = CreateActionSpace(m_HandPoseAction, HAND_LEFT_PATH);
    m_HandSpaces[1] = CreateActionSpace(m_HandPoseAction, HAND_RIGHT_PATH);
}

void OpenXRInputMgr::SetupBindings()
{
    std::vector<std::pair<XrAction, std::string>> simpleControllerBindings = {
            {m_HandPoseAction, "/user/hand/left/input/grip/pose"},
            {m_HandPoseAction, "/user/hand/right/input/grip/pose"},

            {m_SelectAction, "/user/hand/left/input/select/click"},
            {m_SelectAction, "/user/hand/right/input/select/click"},

            {m_HapticAction, "/user/hand/left/output/haptic"},
            {m_HapticAction, "/user/hand/right/output/haptic"}
        };

    AddInteractionProfilerBinding(SIMPLE_CONTROLLER_PROFILE, simpleControllerBindings);

    for (const auto& profileBinding : m_InteractionProfileBindings)
    {
        XrPath interactionProfilePath = XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, profileBinding.interactionProfilePath);

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.next = nullptr;
        suggestedBindings.interactionProfile = interactionProfilePath;
        suggestedBindings.suggestedBindings = profileBinding.bindings.data();
        suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(profileBinding.bindings.size());

        OPENXR_CHECK(xrSuggestInteractionProfileBindings(OpenXRCoreMgr::m_xrInstance, &suggestedBindings),
                     "Failed to suggest bindings for profile: " + profileBinding.interactionProfilePath);

        XR_TUT_LOG("Suggested bindings for interaction profile: " << profileBinding.interactionProfilePath);
    }
}

void OpenXRInputMgr::AddInteractionProfilerBinding(const std::string& interactionProfilePath,
                                                   const std::vector<std::pair<XrAction, std::string>>& actionBindings)
{
    InteractionProfileBinding profileBinding;
    profileBinding.interactionProfilePath = interactionProfilePath;

    for (const auto& binding : actionBindings)
    {
        XrAction action = binding.first;
        const std::string& pathString = binding.second;

        XrPath bindingPath = XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, pathString);
        profileBinding.bindings.push_back({action, bindingPath});
    }

    m_InteractionProfileBindings.push_back(profileBinding);
}


void OpenXRInputMgr::UpdateControllerStates(XrTime predictedTime, XrSpace referenceSpace)
{
    for (int handIndex = 0; handIndex < 2; ++handIndex)
    {
        const std::string& handPath = (handIndex == 0) ? HAND_LEFT_PATH : HAND_RIGHT_PATH;

        controllerStates[handIndex].lastSelectPressed = controllerStates[handIndex].currentSelectPressed;
        controllerStates[handIndex].currentSelectPressed = GetActionStateBoolean(m_SelectAction, handPath);

        controllerStates[handIndex].pose = GetActionStatePose(m_HandPoseAction, handPath, m_HandSpaces[handIndex],
                                                              referenceSpace, predictedTime, &controllerStates[handIndex].poseActive);
    }
}

void OpenXRInputMgr::CreateActionSet(const std::string& actionSetName, const std::string& localizedName, uint32_t priority)
{
    m_ActionSet.actionSetName = actionSetName;
    m_ActionSet.localizedActionSetName = localizedName;
    m_ActionSet.priority = priority;

    XrActionSetCreateInfo actionSetCreateInfo = {};
    actionSetCreateInfo.type = XR_TYPE_ACTION_SET_CREATE_INFO;
    actionSetCreateInfo.next = nullptr;
    strncpy(actionSetCreateInfo.actionSetName, actionSetName.c_str(), XR_MAX_ACTION_SET_NAME_SIZE);
    strncpy(actionSetCreateInfo.localizedActionSetName, localizedName.c_str(), XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
    actionSetCreateInfo.priority = priority;

    OPENXR_CHECK(xrCreateActionSet(OpenXRCoreMgr::m_xrInstance, &actionSetCreateInfo, &m_ActionSet.actionSet),
                 "Failed to create action set: " + actionSetName);

    XR_TUT_LOG("Created action set: " << actionSetName);
}

void OpenXRInputMgr::DestroyActionSet()
{
    OPENXR_CHECK(xrDestroyAction(m_HandPoseAction), "Failed to destroy Hand Pose Action ");
    OPENXR_CHECK(xrDestroyAction(m_SelectAction), "Failed to destroy Select Action ");
    OPENXR_CHECK(xrDestroyAction(m_HapticAction), "Failed to destroy Haptics Action ");

    if (m_ActionSet.actionSet != XR_NULL_HANDLE)
    {
        OPENXR_CHECK(xrDestroyActionSet(m_ActionSet.actionSet), "Failed to destroy action set: " + m_ActionSet.actionSetName);
    }

    m_ActionSet = {};
    m_InteractionProfileBindings.clear();
    XR_TUT_LOG("Destroyed action set");
}

XrAction OpenXRInputMgr::CreateAction(const std::string& actionName, const std::string& localizedName,
                                      XrActionType actionType, const std::vector<std::string>& subactionPaths)
{
    std::vector<XrPath> xrSubactionPaths;
    for (const auto& pathString : subactionPaths)
    {
        XrPath path = XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, pathString);
        xrSubactionPaths.push_back(path);
    }

    XrActionCreateInfo actionCreateInfo = {};
    actionCreateInfo.type = XR_TYPE_ACTION_CREATE_INFO;
    actionCreateInfo.next = nullptr;
    actionCreateInfo.actionType = actionType;
    strncpy(actionCreateInfo.actionName, actionName.c_str(), XR_MAX_ACTION_NAME_SIZE);
    strncpy(actionCreateInfo.localizedActionName, localizedName.c_str(), XR_MAX_ACTION_NAME_SIZE);
    actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(xrSubactionPaths.size());
    actionCreateInfo.subactionPaths = xrSubactionPaths.data();

    XrAction action;
    OPENXR_CHECK(xrCreateAction(m_ActionSet.actionSet, &actionCreateInfo, &action), "Failed to create action: " + actionName);

    XR_TUT_LOG("Created action: " << actionName);
    return action;
}


void OpenXRInputMgr::AttachActionSet()
{
    XrActionSet actionSet = m_ActionSet.actionSet;

    XrSessionActionSetsAttachInfo attachInfo;
    attachInfo.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
    attachInfo.next = nullptr;
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &actionSet;

    OPENXR_CHECK(xrAttachSessionActionSets(OpenXRCoreMgr::xrSession, &attachInfo), "Failed to attach action set to session");

    XR_TUT_LOG("Attached action set to session: " << m_ActionSet.actionSetName);
}

XrSpace OpenXRInputMgr::CreateActionSpace(XrAction poseAction, const std::string& subactionPath)
{
    XrActionSpaceCreateInfo createActionSpaceInfo = {};
    createActionSpaceInfo.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
    createActionSpaceInfo.next = nullptr;
    createActionSpaceInfo.action = poseAction;
    createActionSpaceInfo.poseInActionSpace = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};

    if (!subactionPath.empty())
    {
        createActionSpaceInfo.subactionPath = XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, subactionPath);
    }

    XrSpace actionSpace;
    OPENXR_CHECK(xrCreateActionSpace(OpenXRCoreMgr::xrSession, &createActionSpaceInfo, &actionSpace), "Failed to create action space");

    return actionSpace;
}

void OpenXRInputMgr::DestroyActionSpaces()
{
    OPENXR_CHECK(xrDestroySpace(m_HandSpaces[0]), "Failed to destroy action space");
    OPENXR_CHECK(xrDestroySpace(m_HandSpaces[1]), "Failed to destroy action space");

    XR_TUT_LOG("Destroyed all action spaces");
}

void OpenXRInputMgr::SyncActions()
{
    XrActiveActionSet activeActionSet{m_ActionSet.actionSet, XR_NULL_PATH};

    XrActionsSyncInfo syncInfo;
    syncInfo.type = XR_TYPE_ACTIONS_SYNC_INFO;
    syncInfo.next = nullptr;
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;

    OPENXR_CHECK(xrSyncActions(OpenXRCoreMgr::xrSession, &syncInfo), "Failed to sync actions");
}

bool OpenXRInputMgr::GetActionStateBoolean(XrAction action, const std::string& subactionPath)
{
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.next = nullptr;
    getInfo.action = action;

    if (!subactionPath.empty())
    {
        getInfo.subactionPath = XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, subactionPath);
    }

    XrActionStateBoolean actionState = {};
    actionState.type = XR_TYPE_ACTION_STATE_BOOLEAN;
    actionState.next = nullptr;
    XrResult result = xrGetActionStateBoolean(OpenXRCoreMgr::xrSession, &getInfo, &actionState);

    return XR_SUCCEEDED(result) && actionState.currentState == XR_TRUE && actionState.isActive == XR_TRUE;
}

XrPosef OpenXRInputMgr::GetActionStatePose(XrAction poseAction, const std::string& subactionPath, XrSpace actionSpace, XrSpace referenceSpace,
                                           XrTime predictedTime, bool* isActive)
{
    XrPosef pose = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};

    if (actionSpace == XR_NULL_HANDLE || referenceSpace == XR_NULL_HANDLE)
    {
        *isActive = false;
        return pose;
    }

    XrActionStateGetInfo getInfo = {XR_TYPE_ACTION_STATE_GET_INFO, nullptr, poseAction,
                                    XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, subactionPath)};

    XrActionStatePose actionStatePose = {};
    actionStatePose.type = XR_TYPE_ACTION_STATE_POSE;
    actionStatePose.next = nullptr;
    XrResult result = xrGetActionStatePose(OpenXRCoreMgr::xrSession, &getInfo, &actionStatePose);

    if (!XR_SUCCEEDED(result) || actionStatePose.isActive != XR_TRUE)
    {
        *isActive = false;
        return pose;
    }

    XrSpaceLocation spaceLocation = {};
    spaceLocation.type = XR_TYPE_SPACE_LOCATION;
    spaceLocation.next = nullptr;
    result = xrLocateSpace(actionSpace, referenceSpace, predictedTime, &spaceLocation);

    *isActive = XR_SUCCEEDED(result) && spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT && spaceLocation.locationFlags &
                XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;

    return isActive ? spaceLocation.pose : pose;
}

void OpenXRInputMgr::ApplyHapticFeedback(XrAction hapticAction, const std::string& subactionPath,
                                         float amplitude, XrDuration duration, float frequency)
{
    XrHapticActionInfo hapticActionInfo = {};
    hapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
    hapticActionInfo.next = nullptr;
    hapticActionInfo.action = hapticAction;

    if (!subactionPath.empty())
    {
        hapticActionInfo.subactionPath = XRPathUtils::StringToPath(OpenXRCoreMgr::m_xrInstance, subactionPath);
    }

    XrHapticVibration vibration;
    vibration.type = XR_TYPE_HAPTIC_VIBRATION;
    vibration.next = nullptr;
    vibration.amplitude = amplitude;
    vibration.duration = duration;
    vibration.frequency = frequency;

    OPENXR_CHECK(xrApplyHapticFeedback(OpenXRCoreMgr::xrSession, &hapticActionInfo, reinterpret_cast<const XrHapticBaseHeader*>(&vibration)),
                 "Failed to apply haptic feedback");
}