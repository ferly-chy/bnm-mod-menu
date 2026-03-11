#include <jni.h>
#include <string>
#include <thread>
#include <vector>

#include "logger.h"
#include <BNM/Image.hpp>
#include <BNM/Class.hpp>
#include <BNM/Field.hpp>
#include <BNM/Method.hpp>
#include <BNM/Loading.hpp>
#include <BNM/Property.hpp>
#include "utils.h"


void OnLoaded();

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    BNM::Loading::AddOnLoadedEvent(OnLoaded);
    BNM::Loading::TryLoadByJNI(env);
    return JNI_VERSION_1_6;
}

// FeatureTypes: Toggle, Seekbar, Category
// Examples:
// Toggle:ToggleName:true
// Seekbar:SeekbarName:1_20_10
// Category:CategoryName
extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_android_support_Menu_getFeatureList(JNIEnv *env, jobject thiz) {
    std::string feats[] = {
            "Toggle:Currencies",
            "Toggle:Contacts",
            "Seekbar:Reward:1_20"
    };
    return toJobjectArray(env, feats);
}

struct Feature {
    bool currencies{false};
    bool contacts{false};
    int reward{1};
};

Feature feature{};

extern "C" JNIEXPORT void JNICALL
Java_com_android_support_Menu_valueChange(
        JNIEnv *env,
        jobject thiz,
        jint featIdx,
        jstring featName,
        jint value
) {
    // featIdx: index in feature list
    switch (featIdx) {
        case 0: {
            feature.currencies = value;
            break;
        }
        case 1: {
            feature.contacts = value;
            break;
        }
        case 2: {
            feature.reward = value;
            break;
        }
        default:
            break;
    }
}

void (*old_AddResource)(void *instance, int resourceType, int amount, int reason,
                        int rewardSource);

void new_AddResource(void *instance, int resourceType, int amount, int reason,
                     int rewardSource) {
    return old_AddResource(instance, resourceType, amount * feature.reward, reason, rewardSource);
}

bool (*old_TrySubtractResource)(void *instance, int resourceType, int amount, int reason,
                                void *reasonDetails);

bool new_TrySubtractResource(void *instance, int resourceType, int amount, int reason,
                             void *reasonDetails) {
    if (feature.currencies) {
        return true;
    }

    return old_TrySubtractResource(instance, resourceType, amount, reason,
                                   reasonDetails);
}

void (*old_GiveGift)(void *instance, void *gift);

void new_GiveGift(BNM::IL2CPP::Il2CppObject *instance, BNM::IL2CPP::Il2CppObject *gift) {
    if (feature.currencies) {
        auto GiveGift = GetMethod<BNM::IL2CPP::Il2CppObject *>(instance, "GiveGift", {"expAmount"});
        auto givenExp = GetMethod<int>(gift, "GetGivenExp")();
        GiveGift(givenExp * feature.reward);
        return;
    }
    return old_GiveGift(instance, gift);
}

void (*old_ContactControllerCtor)(void *instance, void *data, void *profile);

void new_ContactControllerCtor(BNM::IL2CPP::Il2CppObject *instance, void *data, void *profile) {
    old_ContactControllerCtor(instance, data, profile);
    if (feature.contacts) {
        GetMethod<void>(instance, "AddTokens")(100);
    }
}


// [Passion Industry](https://www.nutaku.net/games/passion-industry/)
void OnLoaded() {
    LOGI("OnLoaded");
    auto AssemblyCSharp = BNM::Image("Assembly-CSharp");

    auto ResourcesManager = BNM::Class("Game.Scripts.Engine.Profiles.GameResources",
                                       "ResourcesManager",
                                       AssemblyCSharp);
    auto AddResource = ResourcesManager.GetMethod("AddResource", 4);
    auto TrySubtractResource = ResourcesManager.GetMethod("TrySubtractResource", 4);


    auto RelationshipController = BNM::Class("Game.Scripts.Relationship",
                                             "RelationshipController",
                                             AssemblyCSharp);

    auto GiveGift = RelationshipController.GetMethod("GiveGift", {"gift"});


    auto ContactController = BNM::Class("Game.Scripts.Contacts",
                                        "ContactController",
                                        AssemblyCSharp);

    auto ContactControllerCtor = ContactController.GetMethod(".ctor");

    BNM::BasicHook(AddResource, new_AddResource, old_AddResource);
    BNM::BasicHook(TrySubtractResource, new_TrySubtractResource, old_TrySubtractResource);
    BNM::BasicHook(GiveGift, new_GiveGift, old_GiveGift);
    BNM::BasicHook(ContactControllerCtor, new_ContactControllerCtor, old_ContactControllerCtor);

}