#pragma once

// ============================================================================
//  Constantes de reglage de la file d'envoi SysEx.
//  Modifiables ici, sans toucher au reste du code.
// ============================================================================
namespace SC55Config
{
    // Resolution du timer de la file d'envoi (ms).
    constexpr int kQueueTickMs = 5;

    // Delai minimal entre deux messages SysEx quelconques (ms).
    // Le manuel du SC-55 demande au moins 20 ms.
    constexpr int kMinIntervalMs = 25;

    // Pendant un glissement : au plus un message par parametre toutes les ... ms.
    // Entre deux envois, seule la derniere valeur est conservee.
    constexpr int kDragSendIntervalMs = 60;

    // Si la valeur d'un parametre ne change plus pendant ... ms, sa derniere
    // valeur est envoyee tout de suite (sans attendre kDragSendIntervalMs).
    // Doit rester inferieur a kDragSendIntervalMs pour avoir un effet.
    constexpr int kDragEndTimeoutMs = 40;

    // Pause avant le message suivant apres un GS Reset (ms).
    constexpr int kAfterGsResetDelayMs = 100;

    // Au demarrage, le dernier preset est recharge dans l'editeur.
    // true  : il est aussi envoye au SC-55 (environ 3 s de messages SysEx)
    // false : l'editeur l'affiche seulement ; utiliser "Renvoyer l'etat" pour l'envoyer
    constexpr bool kSendPresetOnStartup = false;

    // Quand une part devient muette, un "All Sounds Off" est envoye sur son canal MIDI
    // pour couper les notes deja en cours (sinon elles continueraient a sonner).
    // Ignore si une autre part encore audible partage le meme canal.
    constexpr bool kSilenceHangingNotesOnMute = true;

    // Reperes blancs discrets sur tous les sliders : valeur min, valeur mediane, valeur max.
    constexpr float kSliderMarkAlpha     = 0.55f;   // opacite du blanc (0 = invisible, 1 = plein)
    constexpr float kSliderMarkThickness = 1.0f;    // epaisseur d'un repere (px)
    constexpr float kSliderMarkGap       = 3.0f;    // ecart entre le curseur et un repere lateral (px)
    constexpr float kSliderMarkLength    = 7.0f;    // longueur d'un repere lateral (px)
    constexpr float kSliderMarkOuterTrim = 3.0f;    // retrait de l'extremite exterieure de chaque repere (px)
    constexpr float kSliderMarkInnerTrim = 2.0f;    // rapprochement du bord interieur vers la piste (px)

    // true  : les reperes min et max sont des barres qui traversent la piste
    // false : comme le repere median, ils restent de part et d'autre de la piste
    constexpr bool kSliderEndMarksCrossTrack = true;
}
