# Plan de validation

| ID test | Domaine | Scénario | Résultat attendu |
|---|---|---|---|
| VAL-001 | Vitesse | Trame vitesse nominale | Acceptée, valeur décodée |
| VAL-002 | Température | Trame température nominale | Acceptée, valeur décodée |
| VAL-003 | Pédale | Trame pédale nominale | Acceptée, valeur décodée |
| VAL-004 | Identifiant | ID standard `0x800` | Rejet |
| VAL-005 | Format | Identifiant étendu | Rejet |
| VAL-006 | DLC | DLC = 9 | Rejet |
| VAL-007 | DLC | DLC incorrect pour une vitesse | Rejet critique |
| VAL-008 | Routage | ID non répertorié | Rejet non critique |
| VAL-009 | Plage | Vitesse = 250,1 km/h | Rejet critique |
| VAL-010 | Plage | Température = 151 °C | Rejet critique |
| VAL-011 | Plage | Pédale = 101 % | Rejet critique |
| VAL-012 | Génération | Valeur en entrée hors plage | Exception `std::out_of_range` |
| BUS-001 | Bus | Deux trames successives | Distribution FIFO |
| BUS-002 | Bus | Deux abonnés | Chaque abonné reçoit chaque trame |
| ECU-001 | ECU | Trois trames valides | Télémétrie mise à jour |
| ECU-002 | ECU | ID inconnu | Rejet, ECU opérationnel |
| ECU-003 | ECU | Signal pédale hors plage | Entrée en `SAFE_STATE` |
| ECU-004 | ECU | Trame après `SAFE_STATE` | Trame ignorée |
| INT-001 | Intégration | Bus virtuel → ECU simulé | Compteurs et état attendus |

## Critères d'acceptation

- Compilation C++17 réussie.
- Tous les tests CTest passent.
- Aucun accès matériel ou réseau externe n'est nécessaire.
- Chaque rejet est associé à un diagnostic explicite.
- Les résultats sont reproductibles à environnement logiciel identique.
