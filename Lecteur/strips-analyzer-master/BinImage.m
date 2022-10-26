function ImB=BinImage(Im,L,l)

S=size(Im);

h=S(1);
w=S(2);

hh=floor(h/8);
ww=floor(w/4);
ImT=Im(hh:h-3*hh,ww:w-ww,:);

ST=size(ImT);
h=ST(1);
w=ST(2);

r=floor(h/L);
h2=L*r;
w2=l*r;

dh=floor((h-h2)/2);
dw=floor((w-w2)/2);


Im2=ImT(dh+1:dh+h2,dw+1:dw+w2,:);

temp=zeros(L,l,3);


for i=1:L 
    for j=1:l 
        temp(i,j,1)=mean(mean(Im2((i-1)*r+1:i*r,(j-1)*r+1:j*r,1)));
        temp(i,j,2)=mean(mean(Im2((i-1)*r+1:i*r,(j-1)*r+1:j*r,2)));
        temp(i,j,3)=mean(mean(Im2((i-1)*r+1:i*r,(j-1)*r+1:j*r,3)));
    end
end

ImB=temp;

end