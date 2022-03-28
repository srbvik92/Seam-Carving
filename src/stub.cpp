#define cimg_display 0
#include "CImg.h"
#include "slVector.H"
#include "Seam.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace cimg_library;
using namespace std;

struct energy
{
    energy()
    {
      dy = 0.0;
      dx = 0.0;
    }

    double dy;
    double dx;
};

void CalculateEnergy(vector<energy> &eMatrix, vector<SlVector3> &image, int i, int j, int width, int height);

int main(int argc, char *argv[]) 
{
  CImg<double> input(argv[1]);
  CImg<double> lab = input.RGBtoLab();
  vector<SlVector3> image(input.width() * input.height());

  for (unsigned int i = 0; i < input.width(); i++) 
  {
    for (unsigned int j = 0; j < input.height(); j++) 
    {
      image[i * input.height() + j][0] = lab(i, j, 0);
      image[i * input.height() + j][1] = lab(i, j, 1);
      image[i * input.height() + j][2] = lab(i, j, 2);
    }
  }

  int inWidth = input.width();
  int inHeight = input.height();
  vector<energy> pEnergy(input.width() * input.height());

  //Precalculate energy of all pixels in horizontal and vertical space
  for (unsigned int i = 0; i < input.width(); i++) 
  {
    for (unsigned int j = 0; j < input.height(); j++) 
    {
      // Calculate the energy
      CalculateEnergy(pEnergy, image, i, j, input.width(), input.height());
    }
  }

  // Calculate how many seams to remove
  int hLoops = input.width() - atoi(argv[3]);
  int vLoops = input.height() - atoi(argv[4]);

  // Remove Horizontal and Vertical seams (alternate)
  while (hLoops || vLoops)
  {
    // Remove the horizontal seams
    if (hLoops)
    { 
      Seam seams[inWidth];
      for (int i = 0; i < inWidth; ++i)
      {
        int row = i;
        int col = 0;
        seams[i].add(row, col, pEnergy[row * inHeight + col].dy);

        while(col < inHeight - 1)
        {
          double hLeft;
          double hCenter;
          double hRight;
          double minimum;

          // Check the leftmost edge
          if(row == 0)
          {
            hLeft = -1;
            hCenter = pEnergy[(row) * inHeight + (col + 1)].dy;
            hRight = pEnergy[(row + 1) * inHeight + (col + 1)].dy;
            minimum = min(hCenter, hRight);
          }
          // Check the rightmost edge
          else if(row == inWidth)
          {
            hLeft = pEnergy[(row - 1) * inHeight + (col + 1)].dy;
            hCenter = pEnergy[(row + 1) * inHeight + (col + 1)].dy;
            hRight = -1;
            minimum = min(hCenter, hLeft);
          }
          else
          {
            hLeft = pEnergy[(row - 1) * inHeight + (col + 1)].dy;
            hCenter = pEnergy[(row) * inHeight + (col + 1)].dy;
            hRight = pEnergy[(row + 1) * inHeight + (col + 1)].dy;

            // Get the minimum energy pixel
            minimum = min(min(hLeft, hCenter), hRight);
          }
          // Move to next column
          col++;

          // Move seam to minimum energy pixel
          if(minimum == hLeft)
          {
            row--;
          }
          else if(minimum == hRight)
          {
            row++;
          }

          // Add new pixel location to the seam
          seams[i].add(row, col, pEnergy[row * inHeight+col].dy);
        }
      }

      int minimumSeam = seams[0].cost;
      int minimumIndex = 0;

      for (int i = 0; i < inWidth; ++i)
      {
        if(seams[i].cost < minimumSeam)
        {
          minimumSeam = seams[i].cost;
          minimumIndex = i;
        }
      }

      Seam *seamRef = &seams[minimumIndex];
      for (int i = seamRef->seam.size() - 1; i >= 0; i--)
      {
        // Calculate offset
        int offset = seamRef->coordinates(i).first * inHeight + seamRef->coordinates(i).second;
        
        // Remove the seam from the image
        for (int j = seamRef->coordinates(i).first; j < inWidth - 1; ++j)
        {
          int temp = j * inHeight + seamRef->coordinates(i).second;
          image[temp][0] = image[temp + inHeight][0];
          image[temp][1] = image[temp + inHeight][1];
          image[temp][2] = image[temp + inHeight][2];
        }

        // Calculate the pixel energy again
        CalculateEnergy(pEnergy, image, seamRef->coordinates(i).first, seamRef->coordinates(i).second, inWidth-1, inHeight);
      }
      // Reduce the image width
      inWidth--;

      // Update the number of seams left to remove
      hLoops--;
    }
    // Remove the vertical seams
    if(vLoops)
    {
      Seam seams[inHeight];
      for (int i = 0; i < inHeight; ++i)
      {
        int row = 0;
        int col = i;
        seams[i].add(row, col, pEnergy[row * inHeight + col].dx);
        while (row < inWidth - 1)
        {
          double vUp;
          double vCenter;
          double vDown;
          double minimum;

          // Check the top edge
          if (col == 0)
          {
            vUp = -1;
            vCenter = pEnergy[(row + 1) * inHeight + (col)].dx;
            vDown = pEnergy[(row + 1) * inHeight + (col + 1)].dx;

            minimum = min(vCenter, vDown);
          }
          // Check the bottom edge
          else if (col == inHeight)
          {
            vUp = pEnergy[(row + 1) * inHeight + (col - 1)].dx;
            vCenter = pEnergy[(row + 1) * inHeight + (col + 1)].dx;
            vDown = -1;
            minimum = min(vCenter, vUp);
          }
          else
          {
            vUp = pEnergy[(row + 1) * inHeight + (col - 1)].dx;
            vCenter = pEnergy[(row + 1) * inHeight + (col)].dx;
            vDown = pEnergy[(row + 1) * inHeight + (col + 1)].dx;

            minimum = min(min(vUp, vCenter), vDown);
          }
          // Move to next row
          row++;

          // Move seam to minimum energy pixel
          if(minimum == vUp)
              col--;
          else if(minimum == vDown)
              col++;

          // Add new pixel location to the seam
          seams[i].add(row, col, pEnergy[row * inHeight + col].dx);
        }
      }

      // Calculate minimum seam cost.
      int minimumSeam = seams[0].cost;
      int minimumIndex = 0;
      for (int i = 0; i < inHeight; ++i)
      {
        if(seams[i].cost < minimumSeam)
        {
          minimumSeam = seams[i].cost;
          minimumIndex = i;
        }
      }

      //Create reference to seam
      Seam *seamRef = &seams[minimumIndex];

      for (int i = seamRef->seam.size() -1; i >= 0; i--)
      {
        // Calculate the offset
        int offset = seamRef->coordinates(i).first * input.height() 
            + seamRef->coordinates(i).second;

        // Remove the seam from the image
        for (int j = seamRef->coordinates(i).second; j < inHeight - 1; ++j)
        {
          int temp = seamRef->coordinates(i).first * input.height() + j;
          image[temp][0] = image[temp + 1][0];
          image[temp][1] = image[temp + 1][1];
          image[temp][2] = image[temp + 1][2];
        }

        // Calculate the pixel energy again
        CalculateEnergy(pEnergy, image, seamRef->coordinates(i).first, 
            seamRef->coordinates(i).second, inWidth - 1, input.height());
      }

      // Reduce the image height
      inHeight--;

      // Update the number of seams left to remove
      vLoops--;
    }
  }

  // Print out the image
  CImg<double> output(atoi(argv[3]), atoi(argv[4]), input.depth(), input.spectrum(), 0);
  for (unsigned int i = 0; i < inWidth; i++) 
  {
      for (unsigned int j = 0; j < inHeight; j++) 
      {
          output(i, j, 1) = image[i * input.height() + j][1];
          output(i, j, 2) = image[i * input.height() + j][2];
          output(i, j, 0) = image[i * input.height() + j][0];
      }
  }
  CImg<double> rgb = output.LabtoRGB();

  if (strstr(argv[2], "png"))
      rgb.save_png(argv[2]);
  else if (strstr(argv[2], "jpg"))
      rgb.save_jpeg(argv[2]);

  return 0;
}

// Calculates the energy of pixels
void CalculateEnergy(vector<energy> &eMatrix, vector<SlVector3> &image, 
    int i, int j, int width, int height)
{
  SlVector3 tempVector;
    
  // Calculate energy of pixels for a horizontal seam
  // For the top edge
  if (j == 0)
  {
    eMatrix[i * height + j].dx = mag(image[i * height + j] - image[i * height + j + 1]);
  }
  // For the bottom edge
  else if (j + 1 >= height)
  {
      //Average current pixel with left pixel
      eMatrix[i * height + j].dx = mag(image[i * height + j - 1] - image[i * height + j]);
  }
  else
  {
      //Otherwise calculate surrounding pixels
      eMatrix[i * height + j].dx = mag(image[i * height + j - 1] - image[i * height + j + 1]);
  }

  // Calculate energy of pixels for a horizontal seam
  // For the left edge
  if (i == 0)
  {
    tempVector = image[i * height + j] - image[(i + 1) * height + j];
    eMatrix[i * height + j].dy = (tempVector[0] * tempVector[0]) 
                              + (tempVector[1] * tempVector[1]) 
                              + (tempVector[2] * tempVector[2]);
  }
  // For the right edge
  else if (i + 1 >= width)
  {
    tempVector = image[(i - 1) * height + j] - image[i * height + j];
    eMatrix[i * height + j].dy = (tempVector[0] * tempVector[0])
                          + (tempVector[1] * tempVector[1])
                          + (tempVector[2] * tempVector[2]);
  }
  else
  {
    tempVector = image[(i - 1) * height + j] - image[(i + 1) * height + j];
    eMatrix[i * height + j].dy = (tempVector[0] * tempVector[0]) 
                          + (tempVector[1] * tempVector[1]) 
                          + (tempVector[2] * tempVector[2]);
  }
}