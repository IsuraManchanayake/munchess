# v0.2 - commit 29a30f3

- Search depth is 1
- Evaluates the board based on the piece values. Gives maximum score if the opponent is mate. 0 if stalemate or 50 move rule.


## Benchmark results

### v0.2 vs v0.2 (100 games)

<details open><summary>Summary</summary>

Decisive Games: 26
- Checkmates: 26

Draws: 74
- Three-fold repetition: 7
- Fifty move rule: 1
- Insufficient material: 66
- Stalemate: 0

</details>

<details><summary>Details</summary>

```
Player: Munchess 0.2
   "Draw by 3-fold repetition": 7
   "Draw by fifty moves rule": 1
   "Draw by insufficient mating material": 66
   "Loss: Black mates": 9
   "Loss: White mates": 7
   "Win: Black mates": 4
   "Win: White mates": 6
Player: Munchess 0.2
   "Draw by 3-fold repetition": 7
   "Draw by fifty moves rule": 1
   "Draw by insufficient mating material": 66
   "Loss: Black mates": 4
   "Loss: White mates": 6
   "Win: Black mates": 9
   "Win: White mates": 7
Finished match
```
</details>

### v0.2 vs v0.1 (100 games)

<details open><summary>Summary</summary>

Decisive Games: 97
- Checkmates: 97

Draws: 3
- Three-fold repetition: 0
- Fifty move rule: 0
- Insufficient material: 3
- Stalemate: 0

</details>

<details><summary>Details</summary>

```
Player: Munchess 0.2
   "Draw by 3-fold repetition": 7
   "Draw by fifty moves rule": 1
   "Draw by insufficient mating material": 66
   "Loss: Black mates": 9
   "Loss: White mates": 7
   "Win: Black mates": 4
   "Win: White mates": 6
Player: Munchess 0.2
   "Draw by 3-fold repetition": 7
   "Draw by fifty moves rule": 1
   "Draw by insufficient mating material": 66
   "Loss: Black mates": 4
   "Loss: White mates": 6
   "Win: Black mates": 9
   "Win: White mates": 7
Finished match
```
</details>


# v0.1 - commit 6fab1a1

- Random but legal move generation

## Benchmark results

### v0.1 vs v0.1 (100 games)

<details open><summary>Summary</summary>

Decisive Games: 15
- Checkmates: 15

Draws: 85
- Three-fold repetition: 4
- Fifty move rule: 18
- Insufficient material: 56
- Stalemate: 7
</details>

<details><summary>Details</summary>

```
Player: Munchess 0.1
   "Draw by 3-fold repetition": 4
   "Draw by fifty moves rule": 18
   "Draw by insufficient mating material": 56
   "Draw by stalemate": 7
   "Loss: Black mates": 5
   "Loss: White mates": 5
   "Win: Black mates": 1
   "Win: White mates": 4
Player: Munchess 0.1
   "Draw by 3-fold repetition": 4
   "Draw by fifty moves rule": 18
   "Draw by insufficient mating material": 56
   "Draw by stalemate": 7
   "Loss: Black mates": 1
   "Loss: White mates": 4
   "Win: Black mates": 5
   "Win: White mates": 5
Finished match
```
</details>
