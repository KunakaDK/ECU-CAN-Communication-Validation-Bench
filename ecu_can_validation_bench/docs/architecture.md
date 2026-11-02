# Architecture logicielle

## Flux d'exécution

1. `FrameFactory` encode les valeurs d'entrée nominales dans le format de démonstration.
2. Le producteur place les objets `CanFrame` dans la file FIFO de `VirtualCanBus`.
3. `dispatch()` distribue chaque trame aux composants abonnés.
4. `SimulatedEcu::receive()` appelle `FrameValidator` tant que l'ECU est opérationnel.
5. Le validateur retourne une décision, un code diagnostic, le type de signal, une valeur physique décodée et un indicateur de criticité.
6. L'ECU met à jour la télémétrie si la trame est valide. Si une erreur classée critique est détectée, il passe en `SAFE_STATE`.
7. Dans cet état, les trames suivantes sont comptées et journalisées comme ignorées.

## Décisions de validation

| Contrôle | Résultat |
|---|---|
| ID standard supérieur à `0x7FF` | Rejet |
| Identifiant étendu | Rejet (hors profil de ce simulateur) |
| DLC supérieur à 8 | Rejet |
| ID inconnu | Rejet et journalisation, sans transition de sécurité |
| DLC inattendu pour un ID connu | Rejet critique et entrée en `SAFE_STATE` |
| Signal décodé hors plage | Rejet critique et entrée en `SAFE_STATE` |
| Trame valide | Acceptation et mise à jour de la télémétrie |
| Toute trame après `SAFE_STATE` | Ignorée et journalisée |

Le statut « critique » est une règle pédagogique propre au modèle simulé, et non une analyse de sécurité automobile certifiée.
