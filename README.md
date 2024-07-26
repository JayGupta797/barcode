# Movie Barcode
Self-described engineer, space lover and tea drinker Thomas Poulet defines a movie barcode as follows:

> A Movie Barcode is the color identity card of the movie. Basically for each frame you take the dominant color and create a stripe out of it. The result is an overview of the overall mood of the movie.

It is worth noting that this program computes the row-wise average over a batch of frames rather than the dominant or modal color. In practice, this yields a reasonable effect.

# Approach
Conceptually, my approach uses two key ingredients: a Reader and Process function in alignment with the Producer/Consumer design pattern. The reader is responsible for reading in frames and the workers process a batch of frames into a strip.
![fzNhsbe6](https://github.com/user-attachments/assets/97ce3fba-af51-4152-a349-f67d55b1e90a)

# Template
./barcode [options]
Options:
  -f, --file <file>          Movie file (required)
  -r, --rate <rate>          Sampling rate (default: 1)
  -b, --batches <batches>    Number of batches (default: 100)
  -w, --workers <workers>    Number of workers (default: 5)
  -v, --verbose              Verbose output (default: false)
  -t, --transform            Transform output (default: false)
  -h, --help                 Display this help message

# Example
Here is an example using a [compilation](https://youtu.be/rzNek4MfK5M?si=HRC4kT1_8PAQ-3Jv) of cutscenes from Zelda Breath of the Wild. In post, a filter was applied to brighten colors and [squoosh](https://squoosh.app/) was used to compress the image to a reasonable size.
![vTBflB3o](https://github.com/user-attachments/assets/2ed1a3d0-a1f4-4547-917b-cd8915a1a834)

