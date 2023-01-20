/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Jeffery Shepherd

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    NO
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//



//Adds vignette effect to image (dark corners)
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            
            double distance = sqrt(pow((c - width/2),2) + pow((r - height/2),2));
            double scaling_factor = (height - distance) / height;
            int new_red = red * scaling_factor;
            int new_green = green * scaling_factor;
            int new_blue = blue * scaling_factor;
            
            new_image[r][c].red = new_red;
            new_image[r][c].green = new_green;
            new_image[r][c].blue = new_blue;
        }
    }
    return new_image;
}


//Adds Clarendon effect to image (darks darker and lights lighter) by a scaling factor
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            int average = (red + green + blue) / 3;
            
            if(average >= 170)
            {
                new_image[r][c].red = 255 - (255 - red) * scaling_factor;
                new_image[r][c].green = 255 - (255 - green) * scaling_factor;
                new_image[r][c].blue = 255 - (255 - blue) * scaling_factor;
            }
            else if(average < 90)
            {
                new_image[r][c].red = red * scaling_factor;
                new_image[r][c].green = green * scaling_factor;
                new_image[r][c].blue = blue * scaling_factor;
            }
            else
            {
                new_image[r][c].red = red;
                new_image[r][c].green = green;
                new_image[r][c].blue = blue;
            }
        }
    }
    return new_image;
}


// Grayscale image
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            int gray_value = (red + green + blue) / 3;
                
             new_image[r][c].red = gray_value;
             new_image[r][c].green = gray_value;
             new_image[r][c].blue = gray_value;
        }
    }
    return new_image;
}


// Rotates image by 90 degrees clockwise (not counter-clockwise)
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(width, vector<Pixel> (height));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            
            new_image[c][(height-1)-r].red = red;
            new_image[c][(height-1)-r].green = green;
            new_image[c][(height-1)-r].blue = blue;
        }
    }
    return new_image;
}
    

// Rotates image by a specified number of multiples of 90 degrees clockwise
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    int angle = number * 90;
    
    if(angle%90 != 0)
    {
        cout << "angle must be a multiple of 90 degrees.";
        return image;
    }
    else if(angle%360 == 0)
    {
        return image;
    }
    else if(angle%360 == 90)
    {
        return process_4(image);
    }
    else if(angle%360 == 180)
    {
        return process_4(process_4(image));
    }
    else
    {
        return process_4(process_4(process_4(image)));
    }
}


// Enlarges the image in the x and y direction
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height*y_scale, vector<Pixel> (width*x_scale));
    
    for(int r=0; r<height*y_scale; r++)
    {
        for(int c=0; c<width*x_scale; c++)
        {
            int red = image[r/y_scale][c/x_scale].red;
            int green = image[r/y_scale][c/x_scale].green;
            int blue = image[r/y_scale][c/x_scale].blue;
            
            new_image[r][c].red = red;
            new_image[r][c].green = green;
            new_image[r][c].blue = blue;
        }
    }
    return new_image;
}


// Convert image to high contrast (black and white only)
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            double gray_value = (red + green + blue) / 3;
            
            if(gray_value >= 255/2)
            {
                new_image[r][c].red = 255;
                new_image[r][c].green = 255;
                new_image[r][c].blue = 255;
            }
            else
            {
                new_image[r][c].red = 0;
                new_image[r][c].green = 0;
                new_image[r][c].blue = 0;
            }
        }
    }
    return new_image;
}

    
// Lightens image by a scaling factor
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor)     
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            
            new_image[r][c].red = 255 - (255 - red)*scaling_factor;
            new_image[r][c].green = 255 - (255 - green)*scaling_factor;
            new_image[r][c].blue = 255 - (255 - blue)*scaling_factor;
        }
    }
    return new_image;
}
    
    
// Darkens image by a scaling factor
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)    
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            
            new_image[r][c].red = red * scaling_factor;
            new_image[r][c].green = green * scaling_factor;
            new_image[r][c].blue = blue * scaling_factor;
        }
    }
    return new_image;
}
    
    
// Converts image to only black, white, red, blue, and green
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_image(height, vector<Pixel> (width));
    
    for(int r=0; r<height; r++)
    {
        for(int c=0; c<width; c++)
        {
            int red = image[r][c].red;
            int green = image[r][c].green;
            int blue = image[r][c].blue;
            int max_color;
            
            if(red >= green && red >= blue)
            {
                max_color = red;
            }
            else if(green >= blue)
            {
                max_color = green;
            }
            else
            {
                max_color = blue;
            }
            
            if(red + green + blue >= 550)
            {
                new_image[r][c].red = 255;
                new_image[r][c].green = 255;
                new_image[r][c].blue = 255;
            }
            else if(red + green + blue <= 150)
            {
                new_image[r][c].red = 0;
                new_image[r][c].green = 0;
                new_image[r][c].blue = 0;
            }   
            else if(max_color == red)
            {
                new_image[r][c].red = 255;
                new_image[r][c].green = 0;
                new_image[r][c].blue = 0;
            }
            else if(max_color == green)
            {
                new_image[r][c].red = 0;
                new_image[r][c].green = 255;
                new_image[r][c].blue = 0;
            }
            else
            {
                new_image[r][c].red = 0;
                new_image[r][c].green = 0;
                new_image[r][c].blue = 255;
            }
        }
    }
    return new_image;
}

//////////
//////////
//////////
//////////
//////////
    

int main()
{
    cout << "Image Processing Application" << endl;
    cout << "Enter input BPM filename: " << endl;
    string input_filename;
    cin >> input_filename;
    
    string menu_input = "N";
    
    while(menu_input != "Q")
    {
        cout << "Please choose a process by number (or Q to quit): " << endl;
        cout << "0) Change image (current: " << input_filename <<  ")" << endl;
        cout << "1) Vignette" << endl;
        cout << "2) Clarendon" << endl;
        cout << "3) Grayscale" << endl;
        cout << "4) Rotate 90 degrees" << endl;
        cout << "5) Rotate multiples of 90 degrees" << endl;
        cout << "6) Enlarge" << endl;
        cout << "7) High contrast" << endl;
        cout << "8) Lighten" << endl;
        cout << "9) Darken" << endl;
        cout << "10) Black, white, red, green, and blue only " << endl;
        
        cin >> menu_input;
        
//             quit command
        if(menu_input == "Q")
        {
            cout << "Closing program" << endl;
        }
//             change input file  
        else if(menu_input == "0")
        {
            cout << "Change Image Selected" << endl;
            cout << "Please enter new BMP filename:" << endl;
            cin >> input_filename;
            cout << "Successfully changed input image" << endl;
        }
//             process 1 
        else if(menu_input == "1")
        {
            cout << "Vignette selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_1(image);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied vignette" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 2
        else if(menu_input == "2")
        {
            cout << "Clarendon selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            cout << "Enter scaling factor: " << endl;
            double scaling_factor;
            cin >> scaling_factor;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_2(image, scaling_factor);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied clarendon" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 3
        else if(menu_input == "3")
        {
            cout << "Grayscale selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_3(image);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied grayscale" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 4
        else if(menu_input == "4")
        {
            cout << "Rotate 90 degrees selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_4(image);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully rotated 90 degrees" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 5
        else if(menu_input == "5")
        {
            cout << "Rotate multiples of 90 degrees selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            cout << "Enter number of 90 degree rotations: " << endl;
            int num_rotations;
            cin >> num_rotations;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_5(image, num_rotations);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied rotation of multiples of 90 degrees" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 6
        else if(menu_input == "6")
        {
            cout << "Enlarge selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            cout << "Enter X scale enlargement: " << endl;
            int x_scale;
            cin >> x_scale;
            cout << "Enter Y scale enlargement: " << endl;
            int y_scale;
            cin >> y_scale;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_6(image, x_scale, y_scale);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied enlargement" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 7
        else if(menu_input == "7")
        {
            cout << "High contrast selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_7(image);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied high contrast" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 8
        else if(menu_input == "8")
        {
            cout << "Lighten selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            cout << "Enter scaling factor: " << endl;
            double scaling_factor;
            cin >> scaling_factor;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_8(image, scaling_factor);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied lightening" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 9
        else if(menu_input == "9")
        {
            cout << "Darken selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            cout << "Enter scaling factor: " << endl;
            double scaling_factor;
            cin >> scaling_factor;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_9(image, scaling_factor);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied darkening" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             process 10
        else if(menu_input == "10")
        {
            cout << "Black, white, red, green, and blue only selected" << endl;
            cout << "Enter output filename: " << endl;
            string output_filename;
            cin >> output_filename;
            
            vector<vector<Pixel>> image = read_image(input_filename);
            vector<vector<Pixel>> new_image = process_10(image);
            bool success = write_image(output_filename, new_image);
            
            if(success)
            {
                cout << "Successfully applied black, white, red, green, and blue only" << endl;
            }
            else
            {
                cout << "An error has occurred. Please try again" << endl;
            }
        }
//             invalid input message
        else
        {
            cout << "Please enter a valid menu number or Q to quit" << endl;
        }
    }
    
    return 0;
}