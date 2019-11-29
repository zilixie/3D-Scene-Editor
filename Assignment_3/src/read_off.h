#ifndef READOFF_H
#define READOFF_H

#include <vector>
#include <Eigen/Core>

bool read_off(const std::string filename, Eigen::MatrixXf & V, Eigen::MatrixXi & F);

inline bool read_off(const std::string filename, Eigen::MatrixXf & V, Eigen::MatrixXi & F) {
	FILE * off_fp = fopen(filename.c_str(),"r");
	if (NULL == off_fp) { printf("Open file failed: %s.\n",filename.c_str()); }
	else {
		char buf[500];
		if (fscanf(off_fp,"%s\n",buf) != 1) {
			printf("Failed to read the first line of file.");
			fclose(off_fp);
			return false;
		}
		bool off_hdr = (std::string(buf).compare(0,3,"OFF")==0); //inspect the header could be other than OFF
		if (!off_hdr) {
			printf("Opened file: %s does not have OFF file header.",buf);
			fclose(off_fp);
			return false;
		}
		// get rid of comments
		bool skip = true;
		for (;skip;) {
			fgets(buf,500,off_fp);
			skip = (buf[0] == '\n' || buf[0] == '#');
		}

		int nv, nf, ne;
		sscanf(buf,"%d %d %d",&nv,&nf,&ne);
		V.resize(nv, 3);
		F.resize(nf, 3);

		for (int i = 0; i < nv; i++) {
			float x,y,z;
			fgets(buf, 500, off_fp);
			if(sscanf(buf, "%g %g %g",&x,&y,&z) == 3) {
				V(i,0) = x;
				V(i,1) = y;
				V(i,2) = z;
			} else {
				printf("Read Error: incorrect line format.");
				return false;
			}
		}
		for (int i = 0; i < nf; i++) {
			int n; //could be not 3
			fgets(buf, 500, off_fp);
			int i1, i2, i3;
			if (sscanf(buf, "%d %d %d %d",&n,&i1,&i2,&i3) == 4) {
				F(i,0) = i1;
				F(i,1) = i2;
				F(i,2) = i3;
			} else {
				printf("Read Error: incorrect line format.");
				return false;
			}
		}
		return true;
	}

	fclose(off_fp);
	return false;
}


#endif