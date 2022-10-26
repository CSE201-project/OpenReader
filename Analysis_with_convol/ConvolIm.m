function ImC=ConvolIm(Im,N2)


S=size(Im)
h=S(1);
w=S(2);
 for i=1:h
     for j=1:w
         PSF(i,j)=exp(-((j-round(w/2)).^2)/0.00001)*exp(-((i-round(h/2)).^2)/N2);
     end
 end
 
 PSF=PSF/sum(sum(PSF));

PSFF=ifftshift(fft2(PSF));
ImF=ifftshift(fft2(Im));
ImCF=ImF.*PSFF;
ImC=real(fftshift(ifft2(fftshift(ImCF))));
