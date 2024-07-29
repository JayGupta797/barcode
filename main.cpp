#include <opencv2/opencv.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <vector>
#include <getopt.h>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace {
    constexpr int DEFAULT_RATE = 1;
    constexpr int DEFAULT_BATCHES = 100;
    constexpr int DEFAULT_WORKERS = 5;
    constexpr bool DEFAULT_VERBOSE = false;
    constexpr bool DEFAULT_TRANSFORM = false;
    constexpr char DEFAULT_OUTPUT_FILE[] = "../barcode.png";
}

enum ReturnCode { SUCCESS = 0, FAILURE = 1 };

/*
 * Batch Struct
 * ------------
 * The Batch structure maintains a vector of frames and a unique identifier.
 * This abstraction helps process frames in easy-to-manage groups.
*/
struct Batch {
    int id;
    std::vector<cv::Mat> frames;
};

/*
 * CommandLineArguments Struct
 * ---------------------------
 * The CommandLineArguments structure maintains all barcode options.
 * Missing options are reasonably defaulted.
*/
struct CommandLineArguments {
    std::string file;
    int rate = DEFAULT_RATE;
    int batches = DEFAULT_BATCHES;
    int workers = DEFAULT_WORKERS;
    bool verbose = DEFAULT_VERBOSE;
    bool transform = DEFAULT_TRANSFORM;
    std::string output = DEFAULT_OUTPUT_FILE;
};

/*
 * printUsage()
 * ------------
 * This helper function displays all available barcode options. 
*/
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -f, --file <file>          Movie file (required)\n"
              << "  -r, --rate <rate>          Sampling rate (default: 1)\n"
              << "  -b, --batches <batches>    Number of batches (default: 100)\n"
              << "  -w, --workers <workers>    Number of workers (default: 5)\n"
              << "  -v, --verbose              Verbose output (default: false)\n"
              << "  -t, --transform            Transform output (default: false)\n"
              << "  -h, --help                 Display this help message\n";
}

/*
 * parseArguments()
 * ----------------
 * This helper function parses the supplied command line arguments and updates the structure 
 * (passed by reference) defaults accordingly. Notably, it demands that a file is supplied.
*/
bool parseArguments(int argc, char* argv[], CommandLineArguments& arguments) {
    int opt;
    while ((opt = getopt(argc, argv, "f:r:b:w:o:vth")) != -1) {
        try {
            switch (opt) {
                case 'f':
                    arguments.file = optarg;
                    break;
                case 'r':
                    arguments.rate = std::stoi(optarg);
                    break;
                case 'b':
                    arguments.batches = std::stoi(optarg);
                    break;
                case 'w':
                    arguments.workers = std::stoi(optarg);
                    break;
                case 'o':
                    arguments.output = optarg;
                    break;
                case 'v':
                    arguments.verbose = true;
                    break;
                case 't':
                    arguments.transform = true;
                    break;
                case 'h':
                default:
                    printUsage(argv[0]);
                    return FAILURE;
            }
        } catch (const std::invalid_argument& e) {
            spdlog::error("Invalid argument for option: -{}", static_cast<char>(opt));
            return FAILURE;
        }
    }

    // Check that a video file is supplied
    if (arguments.file.empty()) {
        spdlog::error("A movie file is required.");
        printUsage(argv[0]);
        return FAILURE;
    }

    return SUCCESS;
}


/*
 * checkArguments()
 * ----------------
 * This helper function returns TRUE if the arguments supplied appear valid and false otherwise.
 * Errors are logged for convenience.
*/
bool checkArguments(const CommandLineArguments& arguments) {
    // Check that the movie file is readable and, if so, extract the frame count
    cv::VideoCapture cap(arguments.file);
    if (!cap.isOpened()) {
        spdlog::error("Unable to open the movie file: {}", arguments.file);
        return FAILURE;
    }
    int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);

    // Check that the arguments are positive and do not exceed extreme bounds
    if (arguments.rate <= 0) {
        spdlog::error("Sampling rate must be greater than 0.");
        return FAILURE;
    }
    if (arguments.batches <= 0) {
        spdlog::error("Number of batches must be greater than 0.");
        return FAILURE;
    }
    if (arguments.workers <= 0) {
        spdlog::error("Number of workers must be greater than 0.");
        return FAILURE;
    }
    if (arguments.rate >= totalFrames) {
        spdlog::error("Sampling rate must be less than the total number of frames.");
        return FAILURE;
    }
    int usedFrames = totalFrames / arguments.rate;
    if (arguments.batches >= usedFrames) {
        spdlog::error("Number of batches must be less than the number of used frames.");
        return FAILURE;
    }

    // Validate the output path
    std::filesystem::path path = arguments.output;
    if (!std::filesystem::exists(path.parent_path())) {
        spdlog::error("Invalid output path: {}", arguments.output);
        return FAILURE;
    }

    // Otherwise, release the capture and indicate success
    cap.release();
    return SUCCESS;
}

/*
 * process()
 * ---------
 * This function processes a batch of frames by computing its row-wise average.
 * The result is neatly placed in a results vector based on the batch's id.
*/
void process(const Batch& batch, std::vector<cv::Mat>& results) {
    cv::Mat concatenation;
    cv::Mat average;
    cv::hconcat(batch.frames, concatenation);
    cv::reduce(concatenation, average, 1, cv::REDUCE_AVG);

    // Store the result
    // thread-safe because the unique batch id ensures we never modify the same memory
    results[batch.id] = average;
    spdlog::info("Processed batch {}.", batch.id);
}

/*
 * read()
 * ------
 * This function reads frames from the capture and batches them based on the 
 * sampling rate specified. Once packaged, batches are allocated to worker threads.
*/
void read(boost::asio::thread_pool& pool, std::vector<cv::Mat>& results, const CommandLineArguments& arguments) {
    // Open the video file
    cv::VideoCapture cap(arguments.file);
    if (!cap.isOpened()) {
        spdlog::error("Unable to open the movie file: {}", arguments.file);
        return;
    }

    // Compute frames per batch
    int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);
    int usedFrames = totalFrames / arguments.rate;
    int framesPerBatch = usedFrames / arguments.batches;

    // Setup Counters and Current Batch
    int frameCounter = 0;
    int batchCounter = 0;
    Batch currentBatch{batchCounter};

    // While batches are remaining...
    while (batchCounter < arguments.batches) {
        // Grab frames from the capture
        cap.grab();

        // Retrieve frames based on the sampling rate
        if (frameCounter % arguments.rate == 0) {
            cv::Mat frame;
            cap.retrieve(frame);
            currentBatch.frames.push_back(frame);

            // Allocate the current batch if its size threshold is met
            if (currentBatch.frames.size() == framesPerBatch) {
                boost::asio::post(pool, [currentBatch, &results]() { process(currentBatch, results); });
                spdlog::info("Allocated batch {}.", currentBatch.id);
                batchCounter++;
                currentBatch = Batch{batchCounter};
            }
        }
        frameCounter++;
    }
    spdlog::info("Finished reading frames.");
}

/*
 * polarTransform()
 * ----------------
 * This helper function remaps the barcode (passed by reference) from cartesian to 
 * polar space in-place.
*/
void polarTransform(cv::Mat& barcode, int nBatches) {
    // Prepare for transformation
    cv::Mat flipped;
    cv::Mat polarImage;
    cv::rotate(barcode, flipped, cv::ROTATE_90_CLOCKWISE);
    
    // Apply transformation
    cv::Size dsize(nBatches, nBatches);
    cv::Point2f center(nBatches / 2.0f, nBatches / 2.0f);
    int maxRadius = nBatches / 2.0f;
    cv::warpPolar(flipped, polarImage, dsize, center, maxRadius, cv::WARP_INVERSE_MAP);

    // Create a circular mask to make pixels outside the circle transparent
    cv::Mat mask = cv::Mat::zeros(polarImage.size(), CV_8UC1);
    cv::circle(mask, center, maxRadius, cv::Scalar(255), -1);

    // Apply the mask to make pixels outside the circle transparent
    cv::cvtColor(polarImage, polarImage, cv::COLOR_BGR2BGRA);
    polarImage.setTo(cv::Scalar(0, 0, 0, 0), mask == 0);
    barcode = polarImage;
}

int main(int argc, char** argv) {
    // Parse and Check arguments
    CommandLineArguments arguments;
    if (parseArguments(argc, argv, arguments) == FAILURE) {
        return FAILURE;
    }
    if (checkArguments(arguments) == FAILURE) {
        return FAILURE;
    }
    if (!arguments.verbose) {
        spdlog::set_level(spdlog::level::off);
    }

    // Setup Results and ThreadPool
    std::vector<cv::Mat> results(arguments.batches);
    boost::asio::thread_pool pool(arguments.workers);
    read(pool, results, arguments);
    pool.join();

    // Package the vector into a single matrix
    cv::Mat barcode;
    cv::hconcat(results, barcode);

    // Apply polar transform if requested
    if (arguments.transform) {
        polarTransform(barcode, arguments.batches);
    }

    // Save the barcode image
    cv::imwrite(arguments.output, barcode);
    spdlog::info("Barcode saved as {}", arguments.output);

    return SUCCESS;
}