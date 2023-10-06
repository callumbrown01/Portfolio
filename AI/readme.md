# CITS3301 SEM 2 2023 PROJECT - GYM SUPER MARIO BROS

## Developed by Callum Brown (22985036)

### Environment
Windows 11 x64, Anaconda, Jupyter Notebook

### Implementing the Rule-Based Logic Model
#### Requirements (installed from conda terminal using pip install)
gym-super-mario-bros
gym 
cv2
numpy
string
jupyter (through conda)

#### Steps
1. open conda terminal and install the required packages above
2. open jupyter notebook
3. download zip and unzip it somewhere you can easily navigate to within jupyter
4. navigate to the 'mario_locate_objects' folder in jupyter 
5. open 'Rule-Based Mario AI Agent.ipynb' 
6. run the first code block
7. view mario in action

### Implementing the Reinforcement Learning Model
#### Requirements 
gym-super-mario-bros==7.3.0
gym==0.21.0
stable-baselines3==1.7.0
os 
torch (installed in my case for gpu optimisation but cpu works too, just slower, plenty of 
tutorials for this online)
*cuda==11.8 (if you opt for gpu usage)

#### Steps 
1. open anaconda terminal and create a new environment called 'marioai' and link it to jupyter notebook (tutorials online for this)
2. install the required packages above in the environment through anaconda prompt, note that these are earlier versions so compatibility is important
3. make sure to install cuda before torch if you opt for gpu usage, and that the versions line up
4. within jupyter swap to the marioai kernel you created in 1.
5. open 'PPO Reinforcement Learning Model.ipynb' and run through the code to start training
6. run the first block of code 
---
7. run the second code block if you'd like to start training a fleet of models
8. open the models folder to view pretrained models. you may load and test these models by running the third block of code after editing the number at the end of line one (which indicates the total timesteps used in training)
