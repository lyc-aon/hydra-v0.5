# Steam Palette Extraction

Last updated: 2026-03-06

Primary source image: `Image Inspiration/Oldschool Steam.jpg`

## Method

1. Load the Steam collage directly with Pillow.
2. Segment non-black connected components to isolate the actual Steam window panes.
3. Quantize each pane independently with deterministic median-cut palette extraction.
4. Merge recurring pane colors to identify the real Steam surface palette rather than the black collage background.
5. Extract a separate bright palette for text, borders, and highlights.

Swatch image: `.runtime/steam-palette-swatch-2026-03-06.png`

## Global quantized palette

- `#000000` x 82277
- `#4D5945` x 58147
- `#3E4637` x 43752
- `#656E5B` x 24142
- `#505746` x 23793
- `#57604E` x 19799
- `#4E5644` x 18606
- `#4B5744` x 18577
- `#4A5541` x 18146
- `#3A4232` x 17876
- `#495040` x 17597
- `#505B48` x 16742
- `#414A39` x 14789
- `#8E937F` x 14274
- `#000003` x 10990
- `#3D4735` x 10338
- `#505948` x 8336
- `#030002` x 7365

## Recurring pane palette

- `#4C5844` x 82890
- `#404838` x 50103
- `#505848` x 30733
- `#505844` x 29033
- `#444C3C` x 12801
- `#4C5440` x 12301
- `#505C48` x 9901
- `#404438` x 9863
- `#586050` x 8745
- `#707868` x 8020
- `#747860` x 5162
- `#506048` x 4822

## Bright text / border palette

- `#9EA28C` x 3581
- `#77816F` x 3481
- `#818A78` x 3431
- `#747C6E` x 3086
- `#939277` x 2128
- `#D2D3C0` x 2041
- `#717C67` x 1699
- `#6E7867` x 1348
- `#80826B` x 597
- `#7C7968` x 585

## Pane boxes

- Pane 1: `(213, 5, 413, 164)`
- Pane 2: `(8, 7, 207, 189)`
- Pane 3: `(425, 8, 624, 142)`
- Pane 4: `(427, 154, 628, 321)`
- Pane 5: `(216, 171, 415, 420)`
- Pane 6: `(6, 198, 206, 394)`
- Pane 7: `(428, 335, 628, 484)`
- Pane 8: `(3, 407, 202, 646)`
- Pane 9: `(213, 426, 413, 666)`

## Pane 1 (213, 5, 413, 164)

- `#4D5945` x 7236
- `#515D49` x 5188
- `#4B5542` x 4600
- `#4E5845` x 3688
- `#6E7966` x 3612
- `#4C5945` x 3553

## Pane 2 (8, 7, 207, 189)

- `#4D5945` x 11707
- `#4E5845` x 6334
- `#525E4A` x 4822
- `#454E3E` x 2941
- `#4C5945` x 2898
- `#505948` x 2795

## Pane 3 (425, 8, 624, 142)

- `#3E4637` x 7057
- `#4F5847` x 4364
- `#56604E` x 3853
- `#767E6E` x 2888
- `#4A5542` x 2838
- `#464F3E` x 2321

## Pane 4 (427, 154, 628, 321)

- `#3E4737` x 8242
- `#4F5845` x 5693
- `#505A47` x 5332
- `#4C5743` x 3917
- `#70735A` x 3325
- `#4D5442` x 2871

## Pane 5 (216, 171, 415, 420)

- `#3F4536` x 9863
- `#4F5745` x 8525
- `#505A48` x 7836
- `#4B5643` x 6516
- `#737762` x 5162
- `#3A4334` x 4493

## Pane 6 (6, 198, 206, 394)

- `#3E4637` x 9023
- `#4F5846` x 6758
- `#4C5844` x 5149
- `#57614E` x 4892
- `#4C5442` x 4830
- `#7B8371` x 4120

## Pane 7 (428, 335, 628, 484)

- `#4D5945` x 12152
- `#4E5744` x 4793
- `#454E3B` x 3238
- `#545D49` x 2841
- `#787F66` x 2661
- `#505946` x 1731

## Pane 8 (3, 407, 202, 646)

- `#3E4637` x 18826
- `#454D3D` x 12801
- `#636C5A` x 4806
- `#3C4635` x 3691
- `#373E2E` x 3611
- `#3E4639` x 1911

## Pane 9 (213, 426, 413, 666)

- `#4D5945` x 19146
- `#4D5744` x 10616
- `#414A39` x 5044
- `#525D4A` x 4713
- `#717A68` x 4408
- `#505948` x 1917
