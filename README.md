# Movie Barcode
Self-described engineer, space lover and tea drinker Thomas Poulet defines a movie barcode as follows:

> A Movie Barcode is the color identity card of the movie. Basically for each frame you take the dominant color and create a stripe out of it. The result is an overview of the overall mood of the movie.

It is worth noting that this program computes the row-wise average over a batch of frames rather than the dominant or modal color. In practice, this yields a reasonable effect.

# Approach
Conceptually, my approach uses two key ingredients: a Reader and Process function in alignment with the Producer/Consumer design pattern. The reader is responsible for reading in frames and the workers process a batch of frames into a strip.

![fzNhsbe6](https://github.com/user-attachments/assets/7b1e2baf-6373-4d01-8fda-581ce40aac20)

## Options

| Option                       | Description                                  | Default           |
|------------------------------|----------------------------------------------|-------------------|
| `-f, --file <file>`          | Movie file (required)                        | -                 |
| `-r, --rate <rate>`          | Sampling rate                                | 1                 |
| `-b, --batches <batches>`    | Number of batches                            | 100               |
| `-w, --workers <workers>`    | Number of workers                            | 5                 |
| `-v, --verbose`              | Verbose output                               | false             |
| `-t, --transform`            | Transform output                             | false             |
| `-h, --help`                 | Display help message                         | -                 |

To generate a barcode with the default settings:

```sh
./barcode -f example.mp4
```

To generate a barcode with a sampling rate of 2, 50 batches, and 10 workers:

```sh
./barcode -f example.mp4 -r 2 -b 50 -w 10
```

To enable verbose output and apply a transform to the output:

```sh
./barcode -f example.mp4 -v -t
```

Feel free to customize further based on your project's requirements and additional information you may want to include.

# Example
Here is an example using a [compilation](https://youtu.be/rzNek4MfK5M?si=HRC4kT1_8PAQ-3Jv) of cutscenes from Zelda Breath of the Wild. In post, a filter was applied to brighten colors and [squoosh](https://squoosh.app/) was used to compress the image to a reasonable size.

![vTBflB3o](https://github.com/user-attachments/assets/2ed1a3d0-a1f4-4547-917b-cd8915a1a834)

