__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void conv2lum( __read_only image2d_t srcImg, __write_only image2d_t destImg)
{
   int2 coords = { get_global_id(0),get_global_id(1) };
   //Attention to RGBA order
   float4 color = read_imagef(srcImg, sampler, coords);
    float4 val = (float4)(0.2126 * color.x + 0.7152 * color.y + 0.0722 * color.z);

   write_imagef(destImg, coords, val);
}
