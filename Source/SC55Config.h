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
}
