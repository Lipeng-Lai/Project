# View Transformation

## 0. Basic Knowledge
### 0.1 Translation
For general situation, such as, we want move the position $(x, y)$ to $(x', y')$.
$$
x' = x + x_t \\
y' = y + y_t
$$
and for the matrix transformation, we have:
$$
x' = m_{11} x + m_{12} y \\
y' = m_{21} x + m_{22} y
$$
now, we can get the transformation matrix:
$$
\begin{bmatrix}x'\\y'\\1\end{bmatrix}=\begin{bmatrix}m_{11}&m_{12}&x_t\\m_{21}&m_{22}&y_t\\0&0&1\end{bmatrix}\begin{bmatrix}x\\y\\1\end{bmatrix}=\begin{bmatrix}m_{11}x+m_{12}y+x_t\\m_{21}x+m_{22}y+y_t\\1\end{bmatrix}
$$
where the '1' in $[x', y', 1]$, It represents the meaning of physical meaning as a 'point', the $[x, y, 1]$ have the same meaning. For special circumstances, we want make the positon of the camera $(x, y)$ move to $(0, 0)$
$$
m_{11} x + m_{12}y + x_t = 0 \\
m_{21} x + m_{22}y + y_t = 0
$$
where, $m_{12} = 0, m_{21} = 0$ and $m_{11} = m_{22} = 1, x_t = -x, y_t = -y$, and it is the simple way make the equation equality. So the translation matrix M is:
$$
M = \begin{bmatrix}
1 & 0 & -x \\
0 & 1 & -y \\
0 & 0 & 1
\end{bmatrix}
$$

### 0.2 Scale
Such as, we want make the point $(x, y)$ scale to $(\frac12x, \frac12 y)$, and same to the (1)Translation, just make the $x_t = y_t = 0$, and the $m_{11}, m_{22}, m_{ii}$ represent the Scaling multiple. Just like,
$$
x' = m_{11}x \\
y' = m_{22}y \\
...
$$
the scale matrix S:
$$
S = \begin{bmatrix}
m_{11} & 0 & 0 \\
0 & m_{22} & 0 \\
0 & 0 & 1
\end{bmatrix}
$$

### 0.3 Rotation
In the plane right -angle coordinate system, we define a vector $\vec{a}$, which is $\alpha$ with the X -axis angle, and $|\vec{a}| = r$, so we simply can get that:
$$
x_a = r\cos \alpha \\
y_a = r\sin \alpha
$$
Now, rotate the $\vec{a}$ vector $\varphi$ angle, we can get the vector $\vec{b}$:
$$
\begin{aligned}x_b&=r\cos(\alpha+\varphi)=r\cos\alpha\cos\varphi-r\sin\alpha\sin\varphi,\\y_b&=r\sin(\alpha+\varphi)=r\sin\alpha\cos\varphi+r\cos\alpha\sin\varphi.\end{aligned}
$$
Substitute into $r\cos \alpha = x_a, r\sin \alpha = y_b$:
$$
\begin{aligned}x_b&=x_a\cos\varphi-y_a\sin\varphi,\\y_b&=y_a\cos\varphi+x_a\sin\varphi.\end{aligned}
$$
So, we can get the rotation matrix R:
$$
\mathrm{rotate}(\varphi)=\begin{bmatrix}\cos\varphi&-\sin\varphi\\\sin\varphi&\cos\varphi\end{bmatrix}
$$
**It can be shown that "rotation matrix are orthogonal matrix"$^{[3]}$**


## 1. Modeling transformation
Moving camera position to orgin, for example the camera
Use the basic matrix transformation (translation, rotation) to adjust the object to the place where we want.


## 2. Camera transformation
The position of camera is $\vec{e}$ in virtual world, look-at/gaze direction is $\vec{g}$, up direction is $\vec{t}$.
And then, for camera positon, we move the virtual world to real world, 
just $\vec{g} \rightarrow -Z, \vec{t} \rightarrow Y$, so the position of X-axis is $ \vec{g} \times \vec{t} = X$

1. Move the camera position $\vec{e}$ to the origin $(0, 0, 0)$ in real world
2. Re-coordinate the coordinates of the two by rotating matrix

Translating the coordinates of the virtual world into the real world is complicated, but the opposite is simple, and we consider its inverse rotation: $X \rightarrow (g \times t), Y \rightarrow t, Z \rightarrow -g$, just like:

$$
R_{\text{view}}^{-1} = \begin{bmatrix}x_{\hat{g}\times\hat{t}}&x_t&x_{-g}&0\\y_{\hat{g}\times\hat{t}}&y_t&y_{-g}&0\\z_{\hat{g}\times\hat{t}}&z_t&z_{-g}&0\\0&0&0&1\end{bmatrix}
$$
For example, we want $(1, 0, 0)$ in real world(XYZ axis) turn to virtual world($egt$ axis):
$$
\begin{bmatrix}x_{\hat{g}\times\hat{t}}&x_t&x_{-g}&0\\y_{\hat{g}\times\hat{t}}&y_t&y_{-g}&0\\z_{\hat{g}\times\hat{t}}&z_t&z_{-g}&0\\0&0&0&1\end{bmatrix} \times \begin{bmatrix}
1 \\ 0 \\ 0 \\ 0
\end{bmatrix} = \begin{bmatrix}
x_{\hat{g}\times\hat{t}} \\
y_{\hat{g}\times\hat{t}} \\
z_{\hat{g}\times\hat{t}} \\
0
\end{bmatrix}
$$
So, the $R_{\text{view}}^{-1}$ is correct, and **It can be shown that "rotation matrix are orthogonal matrix"$^{[3]}$**, we get the rotation matrix $R_{\text{view}}$:
$$
R_{\text{view}}=\begin{bmatrix}x_{\hat{g}\times\hat{t}}&y_{\hat{g}\times\hat{t}}&z_{\hat{g}\times\hat{t}} & 0\\x_{t}&y_{t}&z_{t}&0\\x_{-g}&y_{-g}&z_{-g} & 0\\0&0&0&1\end{bmatrix}
$$

$$
T_{view}=\begin{bmatrix}1&0&0&-x_e\\0&1&0&-y_e\\0&0&1&-z_e\\0&0&0&1\end{bmatrix}
$$

$$
\mathbf{M}_{\mathrm{cam}}= R_{\text{view}} T_{\text{view}}
$$
The rightmost matrix represents the first move to the origin, and the next matrix represents the second move to overlap the coordinate system.

## 3. Projection transformation

### 3.1 Orthographic Projection transformation
The relative position of the coordinates of the orthogonal projection will not change, all the rays will propagate in parallel, and we only need to convert all the objects into one space.
1. Move the lower left corner of the original space to the origin
2. Magnify the given multiple
3. Move the scaled space range to the new space range

$$
\mathbf{M}_{\text{orth}}=\begin{bmatrix}\frac{2}{r-l}&0&0&-\frac{r+l}{r-l}\\0&\frac{2}{t-b}&0&-\frac{t+b}{t-b}\\0&0&\frac{2}{n-f}&-\frac{n+f}{n-f}\\0&0&0&1\end{bmatrix}\begin{bmatrix}1&0&0&-\frac{r+l}{2}\\0&1&0&-\frac{t+b}{2}\\0&0&1&-\frac{n+f}{2}\\0&0&0&1\end{bmatrix}
$$
where, $r$ is right plane for $X$, $l$ is left plane for $X$, $t$ is top plane for $Y$, $b$ is bottom plane for $Y$, $n$ is near plane for $Z$, $f$ is far plane for $Z$.

### 3.2 Perspective Projection transformation
Perspective projection is the most similar to the way the human eye sees things, following the near large far small.
Perspective projection for a point $(x, y, z)$, and its coordinate turns to $(x', y', z')$, according to similar triangle properties, 
$$
\begin{pmatrix}x\\y\\z\\1\end{pmatrix}\Rightarrow\begin{pmatrix}nx/z\\ny/z\\\text{unknown}\\1\end{pmatrix} \Rightarrow \begin{pmatrix}
nx \\
ny \\
\text{unknown} \\
z
\end{pmatrix}
$$
and then, we can write the translation matrix $M_{\text{persp}}$:
$$
M_{persp\to ortho}=\begin{pmatrix}n&0&0&0\\0&n&0&0\\?&?&?&?\\0&0&1&0\end{pmatrix}
$$
beacause the position of $Z$ is uncorrelated with $x, y$, so the 3th row is:
$$
(0, 0, A, B)
$$
For the near plane, we have:
$$
An + B = n^2 
$$
For the far plane, we have:
$$
Af + B = f^2
$$
So, it is simple to solve the equation:
$$
\begin{cases}
A = n + f \\
B = -nf
\end{cases}
$$
Finally, transformation matrix is:
$$
M_{persp\to ortho}=\begin{pmatrix}n&0&0&0\\0&n&0&0\\0&0&n+f&-nf\\0&0&1&0\end{pmatrix}
$$


## Viewport transformation
Just make the range of $[-1, -1]^2 \rightarrow  [0, width], [0, height]$, so it is simple to get $M_{\text{viewport}}$ :

$$
M_{viewport}=\begin{pmatrix}\frac{width}{2}&0&0&\frac{width}{2}\\0&\frac{height}{2}&0&\frac{height}{2}\\0&0&1&0\\0&0&0&1\end{pmatrix}
$$

After the above four transformations, we have successfully converted any visual object in the game world to the screen:
$$
M = M_{\text{viewport}} M_{\text{Per}} M_{\text{camera}} M_{\text{model}} \\
M_{\text{Per}} = M_{\text{Orthographic}} M_{\text{Perspective}}
$$

# Reference
[1]https://blog.csdn.net/qq_38065509/article/details/105156501
[2]https://blog.csdn.net/qq_38065509/article/details/105309174?spm=1001.2014.3001.5506 $\\$
[3]https://zhuanlan.zhihu.com/p/419854977 $\\$
[4]https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html