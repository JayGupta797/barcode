# Movie Barcode
Self-described engineer, space lover and tea drinker Thomas Poulet defines a movie barcode as follows:

> A Movie Barcode is the color identity card of the movie. Basically for each frame you take the dominant color and create a stripe out of it. The result is an overview of the overall mood of the movie.

It is worth noting that this program computes the row-wise average over a batch of frames rather than the dominant or modal color. In practice, this yields a reasonable effect.

# Approach
Conceptually, my approach uses two key ingredients: a Reader and Process function in alignment with the Producer/Consumer design pattern. The reader is responsible for reading in frames and the workers process a batch of frames into a strip.

## Options

## Options

<table width="100%">
  <thead>
    <tr>
      <th width="30%">Option</th>
      <th width="50%">Description</th>
      <th width="20%">Default</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td width="30%"><code>-f, --file &lt;file&gt;</code></td>
      <td width="50%">Movie file (required)</td>
      <td width="20%">-</td>
    </tr>
    <tr>
      <td width="30%"><code>-r, --rate &lt;rate&gt;</code></td>
      <td width="50%">Sampling rate</td>
      <td width="20%">1</td>
    </tr>
    <tr>
      <td width="30%"><code>-b, --batches &lt;batches&gt;</code></td>
      <td width="50%">Number of batches</td>
      <td width="20%">100</td>
    </tr>
    <tr>
      <td width="30%"><code>-w, --workers &lt;workers&gt;</code></td>
      <td width="50%">Number of workers</td>
      <td width="20%">5</td>
    </tr>
    <tr>
      <td width="30%"><code>-v, --verbose</code></td>
      <td width="50%">Verbose output</td>
      <td width="20%">false</td>
    </tr>
    <tr>
      <td width="30%"><code>-t, --transform</code></td>
      <td width="50%">Transform output</td>
      <td width="20%">false</td>
    </tr>
    <tr>
      <td width="30%"><code>-h, --help</code></td>
      <td width="50%">Display this help message</td>
      <td width="20%">-</td>
    </tr>
  </tbody>
</table>

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

